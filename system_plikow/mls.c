#include "VFS.h"
#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 


int main(int argc, char* argv[])
{
	int ret;
	if(argc != 2)
	{
		printf("ARGUMENT ERROR!\n");
		return 0;
	}
	
	ret = list_VFS_rootdir(argv[1]);
	
	switch(ret)
	{
		case 0:
			break;
		default:
			printf("Error occured while listing directory, code: %d\n", ret);
			return 0;
	}

	return 0;
}
