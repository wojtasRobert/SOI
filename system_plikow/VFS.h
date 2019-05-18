#ifndef SOI_VFS_H_
#define SOI_VFS_H_

#include <stdio.h> 
#include "types.h"

#define BLOCK_SIZE 4096
#define FILENAME_LENGTH (31 - sizeof(block_index_type) - sizeof(size_type)) 

//kazdy blok danych zawiera czterobajtowy indeks		
#define FILE_DATA_CHUNK_SIZE (BLOCK_SIZE - sizeof(block_index_type))	


// STRUKTURA DYSKU:
// [ superblok ] [katalog z deskryptorami] [ bitowa mapa zajetosci ] [ bloki danych ]
// superblok to zawsze jeden blok
// mozna definiowac rozmiar katalogu poprzez wadrtosc zdefiniowana MAX_FILECOUNT
// rozmiar mapy zalezy od ilosci blokow danych
// ilosc blokow danych zalezy od ustawienia superblock.size



// size = 1 blok
struct VFS_superblock
{
	size_type n_datablocks;	// size_type == uint32_t
};

// size = 1 blok
struct VFS_linked_datablock
{
	char data[FILE_DATA_CHUNK_SIZE];
	block_index_type next_block; // next_block == superblock.size <=> nie ma wiecej blokow
};


// size = 32 bajty
struct VFS_filedescriptor
{
	char filename[FILENAME_LENGTH+1];
	size_type size;			//rozmiar w blokach
	block_index_type first_block; 	//pierwszy blok pliku
};

#define MAX_FILECOUNT (8*BLOCK_SIZE/sizeof(struct VFS_filedescriptor)) 

// size = 8 blokow
struct VFS_rootdir
{
	struct VFS_filedescriptor files[MAX_FILECOUNT];
};

// funkcje
int open_and_read_VFS(char* filename, struct VFS_superblock* sblock, struct VFS_rootdir* rdir, bitmap* fu_map, FILE** fd, size_type* d_fs);
int create_VFS(char* filename, size_type size_in_bytes);
int read_VFS_info(char* filename);
int list_VFS_rootdir(char* filename);


#endif
