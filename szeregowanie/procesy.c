#include <sys/types.h>
#include <unistd.h>
#include "/usr/include/lib.h"
#include "/usr/include/stdio.h"
#include <sys/times.h>
#include <time.h>
#include "stdlib.h"
#include "stdio.h"
#include <sys/wait.h>
#include <math.h>

#define TRUE	1

int get_prop(void);
int get_group(void);

int main(int argc, char * argv[])
{
	int i,k,cpid,children,status;
	int prop[2];

	int ppid = getpid();
	int cps = CLOCKS_PER_SEC;
	char pgroup,cgroup;
	clock_t dub;
	double j, cutime_sum, child_time, timee;
	struct tms t;
	time_t start, end;

	if(get_group() == 0) pgroup = 'A';
	else pgroup = 'B';

	prop[0] = get_prop();
	prop[1] = 100 - prop[0];
	
	printf("\n################################################\n");
	printf("\nProporcje czasowe:\nA: %d\tB: %d\n", prop[0], prop[1]);	

	printf("\nJestem rodzicem, pid: %d, grupa: %c\n", ppid, pgroup);
	
	i = 0;
	j = 0;
	k = 0;

	children = atoi(argv[1]);

	while(++i<=children)
	{	
		if(fork() == 0)      /* stworzenie procesu dziecka */
		{	
			start = time(NULL);
			cpid = getpid();
			if(get_group() == 0) cgroup = 'A';
			else cgroup = 'B';
			printf("Jestem dzieckiem, pid: %d, grupa: %c\n", cpid, cgroup);
			while(k<100000)
			{
				j = sin(k);
				k++;
			}	
			end = time(NULL);					
			printf("Wykonalem sie, pid: %d	czas: %f s,	", cpid, difftime(end,start));
			exit(0);
		}		
	}
	
	cutime_sum = 0;		/* zliczana bedzie suma czasow wykonania wszystkich procesow-dzieci */

	while(TRUE)
	{
		if ((cpid = wait(&status)) == -1)	/* oczekuje na zakonczenie pracy procesow potomnych */
		{
			printf("wait() error\n");  /* jak skoncza sie dzieci to tutaj przejdziemy */	
			printf("\n################################################\n");
			return 0;		
		} 
		else if (!WIFEXITED(status))
		{ 
			printf("Child did not exit successfully");
			return 0;
		} 
		else if ((dub = times(&t)) == -1) 
		{ 
			printf("huj");
			return 0;
		}
		else 
		{	child_time = (((double)t.tms_cutime/cps) - cutime_sum);		/* czas dziecka, kazdego z osobna */
			printf("user time: %f s\n", child_time);	
			cutime_sum = cutime_sum + child_time;
			continue;
		}
	}

	
	return 0;
}



/*------------------------------------------------------------------------*/

int get_prop()
{
	message m;
	return _syscall(0,82,&m);
}

int get_group()
{
	message m;
	return _syscall(0,78,&m);
}
