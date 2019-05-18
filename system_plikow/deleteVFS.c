#include "VFS.h"
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>

/*
 *	funkcja usuwajaca plik z dyskiem wirtualnym
 *	arg 1. - nazwa dysku do usuniecia
 */


int main(int argc, char* argv[])
{
	int ret = 0;
	if(argc != 2)
	{
		printf("ARGUMENT ERROR!\n");
		return 0;
	}
	
	if(unlink(argv[1]) != 0)
	{
		printf("Couldn't remove file: %s\n", argv[1]);
	}
	else
	{
		printf("Successfully removed file: %s\n", argv[1]);
	}
	
	return 0;
}

