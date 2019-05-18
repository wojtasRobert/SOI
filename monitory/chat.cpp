#include "monitor.h"
#include <stdio.h>
#include <forward_list>
#include <pthread.h>
#include <string>
#include <iostream>

using namespace std;

#define MSG_LENGTH 25
#define BUFFER_SIZE 10

struct Message	// struktura wiadomosci
{
	char m[MSG_LENGTH];
	int pri;
	int source;
};

class Buffer : public Monitor
{
	public:
		Buffer(unsigned int cap) { CAPACITY = cap; }

		void send_msg(Message m)
		{
			enter();

			if(get_size() == CAPACITY)
				wait(full);

			if(m.pri == 1)
			{
				next_pri = find_next_pri();
				buffer.insert_after(next_pri, m);
			}
			else
			{
				last = find_last();
				buffer.insert_after(last, m);			
			
			}

			if(get_size() == 1)
				signal(empty);

			printf("----------------------------------> Uzytkownik %d wyslal wiadomosc.\n", m.source);	

			leave();
		}

		Message read_msg()
		{
			Message ret;	
			enter();
			
			if(get_size() == 0)
				wait(empty);
			
			ret = *buffer.begin();	// ciekawe czy to dziala
			buffer.pop_front();
				
			if(get_size() == CAPACITY - 1)
				signal(full);		
				
			leave();

			return ret;
		}
		
		forward_list<Message>::iterator find_next_pri()
		{
			forward_list<Message>::iterator i = buffer.begin();
			int counter = 0;
			
			for(i; i != buffer.end(); i++)
			{
				if(i->pri == 1) counter++;
			}
			
			if(counter == 0)
			{
				i = buffer.before_begin();
			}
			else 
			{
				counter = counter - 1;
				i = buffer.begin();	
				for(int j = 0; j < counter; j++) i++;
			}			
			return i;
		}
		
		forward_list<Message>::iterator find_last()
		{
			forward_list<Message>::iterator i = buffer.begin();
			int counter = 0;
			
			for(i; i != buffer.end(); i++)
			{
				counter++;
			}
			
			if(counter == 0)
			{
				i = buffer.before_begin();
			}
			else 
			{
				counter = counter - 1;
				i = buffer.begin();	
				for(int j = 0; j < counter; j++) i++;
			}			
			return i;
		}

		int get_size()
		{
			int counter = 0;
			for(forward_list<Message>::iterator iter=buffer.begin(); iter != buffer.end(); iter++) counter++;

			return counter;
		}
		

	protected:
		forward_list<Message> buffer;
		
		Condition full, empty;
		
		forward_list<Message>::iterator next_pri;
		forward_list<Message>::iterator last;
		unsigned int CAPACITY;
};


int how_many_reg;
int how_many_pri;


Buffer* buffer = new Buffer(BUFFER_SIZE);	// bufor globalny


void set_msg(struct Message *m, int pri, int src, char *msg)	// ustawienie wiadomosci
{
	m->pri = pri;
	m->source = src;
	strcpy(m->m, msg);
}

void priority_user(int userID)
{
	Message m;
	string ID = to_string(userID);
	int i = 0;

	//konstrukcja wiadomosci
	char msg[MSG_LENGTH] = "Wysylam swoje ID: ";
	strcat(msg, ID.c_str());

	set_msg(&m, 1, userID, msg); 

	while(i < how_many_pri)
	{
		buffer->send_msg(m);
		i++;	
	}

}

void regular_user(int userID)
{
	Message m;
	string ID = to_string(userID);
	int i = 0;
	

	//konstrukcja wiadomosci

	char msg[MSG_LENGTH] = "Wysylam swoje ID: ";
	strcat(msg, ID.c_str());
	set_msg(&m, 0, userID, msg); 

	while(i < how_many_reg)
	{
		
		buffer->send_msg(m);			
		i++;
	}

}

void reader(int how_many)
{
	Message m;
	int size;
	int i = 0;

	printf("Witam jestem czatem!\n");

	while(i < how_many)
	{
		sleep(1);
		m = buffer->read_msg();
		size = buffer->get_size();
		printf("--------------------------\n");
		if(m.pri == 1) printf("Pri User %d: %s.\n", m.source, m.m);
		else printf("User %d: %s.\n", m.source, m.m);
		printf("W buforze: %d\n", size);

		i++;
	}
}


void * priority_user(void * userID_ptr)
{
	int userID = * (int*)userID_ptr; free(userID_ptr);
	priority_user(userID);
}

void * regular_user(void *userID_ptr)
{
	int userID = * (int*)userID_ptr; free(userID_ptr);
	regular_user(userID);
}

void * reading_user(void * how_many_ptr)
{
	int how_many = * (int*)how_many_ptr; free(how_many_ptr);
	reader(how_many);
}


int main(int ArgC, char ** ArgV)
{
	
	int pri, reg, read;
	/*int h_pri = 100;
	int h_reg = 200; */

	// argumenty wywolania:	ile userow pri, ile wiad pri, ile userow reg, ile wiad reg, ile do czytania
	
	if(ArgC != 6) 
	{
		printf("ARGUMENT ERROR\n");
		printf("<liczba pri userow> <liczba pri wiad> <liczba reg userow> <liczba reg wiad> <liczba wiad do przeczytania>\n");
		return 0;
	}	
		
	pri = atoi(ArgV[1]);
	
	reg = atoi(ArgV[3]);
	
	read = atoi(ArgV[5]);

	pthread_t reader;
	pthread_t * regular_users = new pthread_t[reg];
	pthread_t * priority_users = new pthread_t[pri];

	/*pthread_t half_pri, half_reg;


	how_many_pri = 5;
	pthread_create(&half_pri, NULL, priority_user, (void *) new int(h_pri));
	pthread_join(half_pri, NULL);

	how_many_reg = 5;
	pthread_create(&half_reg, NULL, regular_user, (void *) new int(h_reg));
	pthread_join(half_reg, NULL);*/

	how_many_pri = atoi(ArgV[2]);
	how_many_reg = atoi(ArgV[4]);


	/* jesli chcemy zwyklych */	
	for(int i = 0; i < reg; i++)
	{
		pthread_create(&regular_users[i], NULL, regular_user, (void *) new int(i));
	}

	/* jesli chcemy priorytetowych */	
	for(int i = 0; i < pri; i++)
	{
		pthread_create(&priority_users[i], NULL, priority_user, (void *) new int(i+reg));
	}

	sleep(2);	// poczekaj z uruchomieniem czatu

	/* jesli ma czytac to utworz czytelnika */		
	if(read > 0) 
	{
		pthread_create(&reader, NULL, reading_user, (void *) new int(read));	
	}

	/* czekamy na zakonczenie */
	for(int i = 0; i < reg; i++)
	{
		pthread_join(regular_users[i], NULL);
	}
	
	for(int i = 0; i < pri; i++)
	{
		pthread_join(priority_users[i], NULL);
	}

	if(read > 0) pthread_join(reader, NULL);

	return 0;
}
