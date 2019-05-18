#include "VFS.h"
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>


/*
 *	funkcja tworzaca plik reprezentujacy wirtualny system plikow
 *	arg 1. - nazwa pliku
 *	arg 2. - rozmiar dysku
 */
int main(int argc, char* argv[])
{
	int ret = 0;
	if(argc != 3)
	{
		printf("ARGUMENT ERROR!\n");
		return 0;
	}


	ret = create_VFS(argv[1], atoi(argv[2]));

	switch(ret)
	{
		case 0:
			printf("Successfully created VFS in file %s\n", argv[1]);
			return 0;		
		case 1:
			printf("Error occured while opening file, code: %d\n", ret);
			return 0;
		default:
			break;
	}
	
	return 0;
}
