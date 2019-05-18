#include "/usr/include/lib.h"                                             
#include "/usr/include/stdio.h"                                           
#include <sys/types.h>
#include <unistd.h>                                                       
                                                                        
int change_group(void);                                                   
int set_scheduler(int x);
int get_group(void);

/*                                                                        
* Program testujacy podstawowe funkcje wywolan systemowych:              
* -> ustalanie proporcji czasowych przydzialu procesora
* -> zmiane grupy procesu
*/
          
int main(int argc, char * argv[])                                         
{
	int ppid = getpid();
	int ret, ticks;
	int prop[2];
	char grupa;
	  
	if(argc == 1)
	{  
		printf("Brak argumentu!\n");
		return 0;
	}

	if(get_group() == 0) grupa = 'A';
	else grupa = 'B';

	printf("\nPID: %d\nGrupa: %c\n", ppid, grupa);

	prop[0] = get_prop();
	prop[1] = 100 - prop[0];

	printf("\nProporcje czasowe:\nA: %d\tB: %d\n", prop[0], prop[1]);

	ticks = atoi(argv[1]);
	ret = set_scheduler(ticks);

	if (ret == -1)
	{
		printf("\nProces z grupy B nie moze zmieniac proporcji, zmieniam grupe.\n");
		change_group();

		if(get_group() == 0) grupa = 'A';
		else grupa = 'B';

		printf("\nPID: %d\nGrupa: %c\n", ppid, grupa);
		ret = set_scheduler(ticks);

		if (ret == -1) printf("BLAD\n");
		else printf("\nProporcje czasowe zmienione\n");
	
		prop[0] = get_prop();
		prop[1] = 100 - prop[0];

		printf("\nProporcje czasowe:\nA: %d\tB: %d\n", prop[0], prop[1]);

		return 0;
	}

	printf("\nProporcje czasowe zmienione\n");

	prop[0] = get_prop();
	prop[1] = 100 - prop[0];

	printf("\nProporcje czasowe:\nA: %d\tB: %d\n", prop[0], prop[1]);
	return 0;

}

/*-----------------------------------------------------------------*
* funkcje wykonujace syscalle                                     *
*-----------------------------------------------------------------*/
int change_group()
{
	message m;
	return _syscall(0,80,&m);
}

int set_scheduler(int x)
{
	message m;
	m.m1_i1 = x;
	return _syscall(0,79,&m);
}

int get_group()
{
	message m;
	return _syscall(0,78,&m);
}

int get_prop()
{
	message m;
	return _syscall(0,82,&m);
}
