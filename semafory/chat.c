#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <pthread.h>

#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5
#define OFFSET sizeof(struct message)
#define PTR_SIZE sizeof(struct message *)
#define CHAR_PTR_SIZE sizeof(char *) 

struct message
{
	struct message *next;
	int priority;
	char m;
};

struct queue
{
	struct message table[BUFFER_SIZE];
	
};

/* wspoldzielone komorki pamieci przechowujace wskazniki */	
struct message *head_shared;
struct message *tail_shared;
char * actual_shm_shared;

/* wskazniki do wpisania w powyzsze komorki */
struct message *head;
struct message *tail;
char * actual_shm; // wskaznik na miejsce do ewentualnego wpisania do pamieci dzielonej 		!!! zawsze rowny tail + 16 !!!

sem_t* mutex;
sem_t* empty;
sem_t* full;

char * shared_memory();
void sem_create();

void user_A();
void user_B();
void reader();

void add_front(struct message *m);
void add(struct message *m);
char read_msg(void);

void test(int);
void test_chat();
void test_starve();
void test_deadlock();

void pop();
void push_front();
void push();

int main(int argc, char * argv[])
{	
	int buff_id, head_id, tail_id, actual_shm_id;
	key_t key_buff, key_head, key_tail, key_act;
	char *shm;
	int status,w,b,d;

	struct message m,n,buf[2];
	struct message *ptr, *ptr2;

	b = sizeof(head);
	d = sizeof(*head);
	w = sizeof(buf);

	printf("%d %d %d\n",w, b, d);
	

	head = tail = 0;
	

	key_buff = 9999;
	key_head = 9998;
	key_tail = 9997;
	key_act  = 9996;

	buff_id = shmget(key_buff, (BUFFER_SIZE + 1) * OFFSET, IPC_CREAT | 0666);
	head_id = shmget(key_head, PTR_SIZE, IPC_CREAT | 0666);
	tail_id = shmget(key_tail, PTR_SIZE, IPC_CREAT | 0666);
	actual_shm_id = shmget(key_act, CHAR_PTR_SIZE, IPC_CREAT | 0666);	

	shm = shmat(buff_id, NULL, 0); 	// wskaznik na wspoldzielony bufor
	head_shared = shmat(head_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na poczatek kolejki
	tail_shared = shmat(tail_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na koniec kolejki
	actual_shm_shared = shmat(actual_shm_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na miejsce kolejnego wpisania 
	
	// inicjalizacja pamieci zerami
	memset(shm, 0, (BUFFER_SIZE + 1) * OFFSET); 
	memset(head_shared, 0,PTR_SIZE);
	memset(tail_shared, 0,PTR_SIZE);
	memset(actual_shm_shared, 0, CHAR_PTR_SIZE);
	
	head_shared = memcpy(head_shared, head, PTR_SIZE);
	tail_shared = memcpy(tail_shared, tail, PTR_SIZE);
	actual_shm_shared = memcpy(actual_shm_shared, actual_shm, CHAR_PTR_SIZE);

	printf("wspoldzielona glowa: %p\n", head_shared);

	sem_create();				// utworzenie semaforow
	
	/*ptr = memcpy(shm,&buf[0],sizeof(struct message));	
	head = ptr;

	ptr = memcpy(shm + 16,&buf[1],sizeof(struct message));
	head->next = ptr;	*/

	switch(fork())
	{
		case 0:
			user_B();
			printf("cos sie chyba wyslalo\n");
			printf("adres ogona: %p\n", tail);
			printf("adres gdzie wpisywac: %p\n", actual_shm);
			exit(0);
		default:
			break;
	}	
		
	sleep(1);
	
	printf("wspoldzielona glowa: %p\n", head_shared);
	printf("adres ogona: %p\n", tail);
	printf("adres gdzie wpisywac: %p\n", actual_shm);

	switch(fork())
	{
		case 0:
			reader();
			printf("koniec dziecka\n");
			exit(0);
		default:
			break;
	}
	
	while(wait(&status) != -1);
    	printf("Koniec Rodzica\n");
	
	return 0;

}

/*void test_chat()
{
	switch(fork())
	{
		case 0:
			reader();
		default:
			break;
	}

	switch(fork())
	{
		case 0:
			user_B();
		default:
			break;
	}

	switch(fork())
	{
		case 0:
			user_A();
		default:
			break;
	}		
}
*/

char * shared_memory()
{
	
	
	
	
}

void sem_create()
{
	mutex = sem_open("/mutex", O_CREAT, 0666, 1);
	empty = sem_open("/empty", O_CREAT, 0666, BUFFER_SIZE);
	full = sem_open("/full", O_CREAT, 0666, 0);
}

char read_msg(void)
{
	
	struct message *temp, *buf;
	int ret;
	
	ret = 0;

	printf("czytam\n");
	//*ret = *head;	// zwracam pierwszy element kolejki
	printf("cos\n");
	temp = head;	// wskaznik na pierwszy element
	printf("cos2 %p\n", tail); 
	ret = temp->m;
	printf("cos2 %c\n", ret);

	while ( temp->next != 0 )
	{	
		printf("wchodze jak w maslo\n");
		buf = temp->next;		// zapisanie adresu nastepnej komorki	
		*temp = *(temp->next); 		// przeniesienie wartosci nastepnej pod adres komorki wyzej
		temp->next = buf;		// adres nastepnej sie nie zmienia
		temp = temp->next;		// przesuniecie wskaznika na komorke nizej 
	}
	
	printf("cos zwroce\n");
	actual_shm = actual_shm - OFFSET;
	free(temp);
	free(buf);
	
	return ret;
	
}

void reader()
{
	char ret;
	int buff_id, head_id, tail_id, actual_shm_id, i;
	key_t key_buff, key_head, key_tail, key_act;
	char *shm;

	key_buff = 9999;
	key_head = 9998;
	key_tail = 9997;
	key_act  = 9996;

	buff_id = shmget(key_buff, (BUFFER_SIZE + 1) * OFFSET, 0666);
	head_id = shmget(key_head, PTR_SIZE, 0666);
	tail_id = shmget(key_tail, PTR_SIZE, 0666);
	actual_shm_id = shmget(key_act, CHAR_PTR_SIZE, 0666);

	shm = shmat(buff_id, NULL, 0); 	// wskaznik na wspoldzielony bufor
	head_shared = shmat(head_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na poczatek kolejki
	tail_shared = shmat(tail_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na koniec kolejki
	actual_shm_shared = shmat(actual_shm_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na miejsce kolejnego wpisania 

	printf("wspoldzielona glowa: %p\n", head_shared);
	printf("Chat rozpoczety...\n");
	
	i = 0;
	/*while(i<10)
	{*/
		printf("kupa\n");
		// semafory wlatuja
		//sem_wait(full);
		//sem_wait(mutex);
		
		ret = read_msg();

		//sem_post(mutex);
		//sem_post(empty);
	
		printf("kupka\n");
		/*i++;
	}

	/*printf("%c\n", head->m);
	
	printf("%c\n", (head->next)->m);*/
}

void add(struct message *m)
{
	struct message *msg;

	if(head == 0 && tail == 0) 
	{	
		printf("wpisuje\n");
		head = tail = memcpy(actual_shm, m, OFFSET);
		head->next = tail->next = 0;
		actual_shm = actual_shm + OFFSET;
		printf("%c\n", head->m);
		printf("%c\n", tail->m);
		printf("adres ogona: %p\n", tail);
		printf("adres gdzie wpisywac: %p\n", actual_shm);
		return;
	}

	tail->next = memcpy(actual_shm, m, OFFSET);
	tail = tail->next;
	tail->next = 0;
	actual_shm = actual_shm + OFFSET;
}

void user_A()
{
	
}

void user_B()
{
	struct message m;
	char msg[] = {'r', 'o', 'b', 'e', 'r', 't'};

	int buff_id, head_id, tail_id, actual_shm_id, i;
	key_t key_buff, key_head, key_tail, key_act;
	char *shm;

	key_buff = 9999;
	key_head = 9998;
	key_tail = 9997;
	key_act  = 9996;

	buff_id = shmget(key_buff, (BUFFER_SIZE + 1) * OFFSET, 0666);
	head_id = shmget(key_head, PTR_SIZE, 0666);
	tail_id = shmget(key_tail, PTR_SIZE, 0666);
	actual_shm_id = shmget(key_act, CHAR_PTR_SIZE, 0666);

	shm = shmat(buff_id, NULL, 0); 	// wskaznik na wspoldzielony bufor
	head_shared = shmat(head_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na poczatek kolejki
	tail_shared = shmat(tail_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na koniec kolejki
	actual_shm_shared = shmat(actual_shm_id, NULL, 0);	// wskaznik na wspoldzielony wskaznik na miejsce kolejnego wpisania 

	printf("wspoldzielona glowa: %p\n", head_shared);
	i = 0;
	/*while (i < 1)
	{*/	
		m.m = msg[0];
		printf("zaraz cos wysle\n");
		/*sem_wait(empty);
		sem_wait(mutex);*/
	
		add(&m);		

		/*sem_post(mutex);		 	
		sem_post(full);
		/*i++;		
	
	}*/

}

















