#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>


#define BUFFER_SIZE 6
#define USER_A	1
#define USER_B	0

#define QUEUE_KEY 9999
#define FULL_KEY 9998
#define EMPTY_KEY 9997
#define MUTEX_KEY 9996


sem_t* mutex;
sem_t* empty;
sem_t* full;

static struct sembuf buf;

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
        	if(q->head < 0) q->head = BUFFER_SIZE - 1; // zmiana indeksu poczatku bufora
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
	q->head = (q->head + 1) % BUFFER_SIZE;	// nowy indeks poczatku
	q->size--;
	
	return m;
}

void down(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("Opuszczenie semafora");
        exit(1);
    }
}

void up(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("Podnoszenie semafora");
        exit(1);
    }
}

void user_A(int input)
{
	int queue_id, i;	
	struct Message msg, msg1, msg2;
	char message[] = {'R','O','B','E','R','T'};

	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), 0666);	// prosba o dostep do pamieci
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor


	int fullId = semget(FULL_KEY, 1, 0600);
	int emptyId = semget(EMPTY_KEY, 1, 0600);
	int mutexId = semget(MUTEX_KEY, 1, 0600);	
	
	i = 0;
	
	while(i<input)
	{	
		set_msg(&msg,1,USER_A,message[i%6]);
		
		down(emptyId, 0);
		down(mutexId, 0);

		send_msg(queue, msg);
		printf("Uzytkownik A wyslal wiadomosc.\n");

		up(mutexId, 0);
		up(fullId, 0);	
		
		i++;		
	}	
}

void user_B(int input)	
{
	int queue_id, i;	
	struct Message msg;
	char message[] = {'r','o','b','e','r','t'};

	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), 0666);	// prosba o dostep do pamieci
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor

	int fullId = semget(FULL_KEY, 1, 0600);
	int emptyId = semget(EMPTY_KEY, 1, 0600);
	int mutexId = semget(MUTEX_KEY, 1, 0600);
	
	i = 0;
	
	while(i<input)
	{		
		set_msg(&msg,0,USER_B,message[i%6]);	
		
		down(emptyId, 0);
		down(mutexId, 0);
	
		send_msg(queue, msg);
		printf("Uzytkownik B wyslal wiadomosc.\n");
		up(mutexId, 0);
		up(fullId, 0);		
		
		i++;		
	}
}

void reader(int input)
{
	int queue_id, i, size;	
	struct Message msg;

	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), 0666);	// prosba o dostep do pamieci
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor

	int fullId = semget(FULL_KEY, 1, 0600);
	int emptyId = semget(EMPTY_KEY, 1, 0600);
	int mutexId = semget(MUTEX_KEY, 1, 0600);
	
	size = 0;
	i = 0;

	while(i<input)
	{	
	
		down(fullId, 0);
		
		down(mutexId, 0);

		printf("------------------\n");
		msg = read_msg(queue);
		if(msg.pri == 1) printf("Uzytkownik A napisal: %c\n", msg.m);
		else if (msg.pri == 0) printf("Uzytkownik B napisal: %c\n", msg.m);

		up(mutexId, 0);
		up(emptyId, 0);

		size = get_queue_size(queue);
		printf("W buforze: %d\n", size);

		
		i++;	
	}
}



int main(int argc, char * argv[])
{
	int queue_id, status, size, A, B, read;	
	
	queue_id = shmget(QUEUE_KEY, sizeof(struct Queue), IPC_CREAT|0666);	// utworzenie bloku pamieci wspoldzielonej
	struct Queue *queue = (struct Queue*)shmat(queue_id, NULL, 0);	// utworzenie wskaznika na wspoldzielony bufor

	queue_init(queue);

	int fullId = semget(FULL_KEY, 1, IPC_CREAT|IPC_EXCL|0600);	
	semctl(fullId, 0, SETVAL, (int)0);
	
	int emptyId = semget(EMPTY_KEY, 1, IPC_CREAT|IPC_EXCL|0600);
	semctl(emptyId, 0, SETVAL, BUFFER_SIZE);
	
	int mutexId = semget(MUTEX_KEY, 1, IPC_CREAT|IPC_EXCL|0600);
 	semctl(mutexId, 0, SETVAL, (int)1);

	if(argc == 1)
	{
		printf("ARGUMENT ERROR!\n");
		return 0;
	}

	A = atoi(argv[1]);
	B = atoi(argv[2]);
	read = atoi(argv[3]);
	
	
	if( B > 0 )
	{
		switch (fork())
		{
			case 0:
				printf("Uzytkownik B zalogowal sie\n");
				user_B(B);
				exit(0);
			default:
				break;
		}
	}

	if( A > 0)
	{
		switch (fork())
		{
			case 0:
				printf("Uzytkownik A zalogowal sie\n");
				user_A(A);
				exit(0);
			default:
				break;
		}
	}

	if( read > 0)
	{
		switch (fork())
		{
			case 0:
				printf("Witam jestem czatem\n");
				reader(read);
				exit(0);
			default:
				break;
		}
	}
	while(wait(&status) != -1);	

	
	shmdt(queue);

	return 0;
}





