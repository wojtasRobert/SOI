#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <time.h>

#define BUFFER_SIZE 5
#define USER_A	1
#define USER_B	0

#define QUEUE_KEY 9999

/*
#define FULL_KEY 9998
#define EMPTY_KEY 9997
#define MUTEX_KEY 9996
*/

sem_t* mutex;
sem_t* empty;
sem_t* full;

struct Message
{
	int pri;
	int source;
	char m;
};

struct Queue
{
	struct Message table[BUFFER_SIZE];
	int head;
	int tail;
	int size;
};

void queue_init(struct Queue *q)
{
	q->head = 0;
	q->tail = 0;
	q->size = 0;	
}

void set_msg(struct Message *m, int pri, int src, char msg)
{
	m->pri = pri;
	m->source = src;
	m->m = msg;
}

int get_queue_size(struct Queue *q)
{
	return q->size;
}

void send_msg(struct Queue *q, struct Message m)
{
	if(m.pri == 1)
	{
		q->head = (q->head - 1);
        	if(q->head < 0) q->head = BUFFER_SIZE - 1;
        	q->table[q->head] = m;
	}
	else
	{
		q->table[q->tail] = m;
		q->tail = (q->tail + 1) % BUFFER_SIZE;
	}
	q->size++;
}

struct Message read_msg(struct Queue *q)
{	
	struct Message m;

	m = q->table[q->head];
	q->head = (q->head + 1) % BUFFER_SIZE;
	q->size--;
	
	return m;
}

void user_A()
{
	int queue_id, i, size;	
	struct Message msg;
	char message[] = {'R','O','B','E','R','T'};

	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), 0666);
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor

	i = 0;
	
	while(i<2)
	{	
		set_msg(&msg,1,USER_A,message[0]);
		sem_wait(mutex);
		sem_wait(empty); 	

		
		send_msg(queue, msg);

		
        	sem_post(full);
		sem_post(mutex);
		printf("Uzytkownik A wyslal wiadomosc.\n");
		
		i++;		
	}	
}

void user_B()
{
	int queue_id, i, size;	
	struct Message msg;
	char message[] = {'r','o','b','e','r','t'};

	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), 0666);
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor

	//printf("B pisze wiadomosc\n");
	i = 0;
	
	while(i<2)
	{
		
		set_msg(&msg,0,USER_B,message[0]);
		
		sem_wait(mutex);
		sem_wait(empty);	

		send_msg(queue, msg);
		
		sem_post(full);
		sem_post(mutex);

        	printf("Uzytkownik B wyslal wiadomosc.\n");		
		i++;		
	}
}

void reader()
{
	int queue_id, i, size;	
	struct Message msg;

	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), 0666);
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor
	size = 0;
	i = 0;
	//sleep(2);
	while(i<10)
	{
		
		
		sem_wait(full);		// printf("hello\n");	// do testu nieczytania z pustego bufora
		sem_wait(mutex);

		msg = read_msg(queue);

		
		sem_post(empty);
		sem_post(mutex);

	

		if(msg.pri == 1) printf("Uzytkownik A napisal: %c |		%d\n", msg.m, size);
		else if (msg.pri == 0) printf("Uzytkownik B napisal: %c |		%d\n", msg.m, size);
		i++;
	
		
	}
}


int main(int argc, char * argv[])
{
	int queue_id, status, size, t0, t;	
	
	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), IPC_CREAT|0666);
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor

	queue_init(queue);
	size = get_queue_size(queue);
	printf("%d\n", size);
	mutex = sem_open("/mutex", O_CREAT, 0666, 1);
	empty = sem_open("/empty", O_CREAT, 0666, BUFFER_SIZE);
	full = sem_open("/full", O_CREAT, 0666, 0);

	size = sizeof(struct Message);
	printf("%d\n",size);

	t0 = time(0);

	
	switch (fork())
	{
		case 0:
			printf("Uzytkownik B zalogowal sie\n");
			user_B();
			exit(0);
		default:
			break;
	}

	switch (fork())
	{
		case 0:
			printf("Uzytkownik A zalogowal sie\n");
			user_A();
			exit(0);
		default:
			break;
	}

	switch (fork())
	{
		case 0:
			printf("Witam jestem czatem\n");
			reader();
			exit(0);
		default:
			break;
	}
	
	while(wait(&status) != -1);

	/*sem_unlink("/mutex_crit");
	sem_unlink("/empty");
	sem_unlink("/full");
*/
	shmdt(queue);

	return 0;
}





