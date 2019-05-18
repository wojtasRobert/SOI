#include <stdlib.h>
#include <stdio.h> 
#include <libgen.h> 
#include <errno.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <string.h> 

#include "VFS.h"
#include "helpers.h"

// [superblok (1)] [katalog (zalezny od MAX_FILECOUNT)] [bitowa mapa zajetosci (zalezne od blokow danych)] [bloki danych (zalezne od podanego rozmiaru)]

#define BLOCKS_NEEDED_FOR_BYTES(nb) ((((nb) - 1)/BLOCK_SIZE) + 1)					// liczba mozliwych do utworzenia blokow przy podanej liczbie bajtow
#define FILE_CHUNKS_NEEDED_FOR_BYTES(nb) (((nb) - 1)/FILE_DATA_CHUNK_SIZE + 1)				// liczba kawalkow na dane przy podanej liczbie bajtow
#define SUPERBLOCK_OFFSET 0										//superblok jest na poczatku (offset w blokach)

#define ROOTDIR_BLOCK_OFFSET 1										//offset katalogu - od razu po superbloku
#define ROOTDIR_BYTE_OFFSET ROOTDIR_BLOCK_OFFSET*BLOCK_SIZE 						//offset katalogu w bajtach
#define ROOTDIR_SIZE_BLOCKS (BLOCKS_NEEDED_FOR_BYTES(sizeof(struct VFS_rootdir)))			//liczba blokow potrzebnych na przechowywanie katalogu

#define BITMAP_BYTE_OFFSET (ROOTDIR_BYTE_OFFSET+sizeof(struct VFS_rootdir))				//offset mapy w bajtach - znajduje sie za katalogiem
#define BITMAP_BLOCK_OFFSET (ROOTDIR_BLOCK_OFFSET + ROOTDIR_SIZE_BLOCKS)				//offset mapy w blokach
#define BITMAP_SIZE_ACTUAL_BYTES(ndblocks) BITNSLOTS(ndblocks)						//rozmiar mapy w bajtach
#define BITMAP_SIZE_BLOCKS(ndblocks) BLOCKS_NEEDED_FOR_BYTES(BITMAP_SIZE_ACTUAL_BYTES(ndblocks))	//rozmiar mapy w blokach
#define BITMAP_SIZE_DISK_BYTES(ndblocks) BITMAP_SIZE_BLOCKS(ndblocks)*BLOCK_SIZE			//faktyczny rozmiar na dysku

#define DATA_BYTE_OFFSET(ndblocks) (BITMAP_BYTE_OFFSET + BITMAP_SIZE_DISK_BYTES(ndblocks))		//offset blokow danych w bajtach - po mapie zajetosci
#define DATA_BLOCK_OFFSET(ndblocks) (BITMAP_BLOCK_OFFSET + BITMAP_SIZE_BLOCKS(ndblocks))		//offset blokow danych w blokach
#define USABLE_DATA_SIZE_BYTES(ndblocks) ((ndblocks)*FILE_DATA_CHUNK_SIZE)				//rozmiar w bajtach uzytecznych blokow pamieci
#define DATA_SIZE_DISK_BYTES(ndblocks) (ndblocks*BLOCK_SIZE)						//zajmowany obszar pamieci na dysku

#define DATA_ITH_BLOCK_OFFSET(block_nr, ndblocks) (DATA_BYTE_OFFSET(ndblocks) + block_nr*BLOCK_SIZE)	//offset i-tego bloku

#define PREDICTED_VFS_FILE_SIZE(ndblocks) (DATA_BYTE_OFFSET(ndblocks) + DATA_SIZE_DISK_BYTES(ndblocks))	// przewidywany rozmiar VFS na dysku ()

/*
 *	funkcja tworzaca dysk, otwiera plik o zadanej nazwie, tworzy wskaznik, tworzy superblok
 *	zapisuje liczbe potrzebnych blokow dla danego rozmiaru, zapisuje superblok do pliku VFS
 */
int create_VFS(char* filename, size_type size_in_bytes)
{
	FILE* disk_file = fopen(filename, "wb");
	struct VFS_superblock superblock;
	
	if(!disk_file) return 1;
	
	superblock.n_datablocks = FILE_CHUNKS_NEEDED_FOR_BYTES(size_in_bytes);
	
	fwrite(&superblock, sizeof(superblock), 1, disk_file);
	
	fclose(disk_file);
	//ustawienie rozmiaru
	truncate(filename, PREDICTED_VFS_FILE_SIZE(superblock.n_datablocks));
	
	return 0;
}

/*
 *	funkcja otwierajaca plik dysku w celu odczytania informacji o rozmiarze pliku,
 *	ilosci superblokow, katalogu glownym VFS
 *	zwraca: wskaznik na dysk, wskaznik na superblok, wskaznik na katalog, wskaznik na mape zajetosci, rozmiar dysku
 */
int open_and_read_VFS(char* filename, struct VFS_superblock* sblock, struct VFS_rootdir* rdir, bitmap* fu_map, FILE** fd, size_type* d_fs)
{
	FILE* disk_file = fopen(filename, "r+b");	//otworz do odczytu
	struct VFS_superblock superblock;
	struct VFS_rootdir rootdir;
	bitmap free_space_map;
	size_type disk_file_size;
	
	// czy otworzenie pliku sie powiodlo
	if(!disk_file)
	{
		return 1;
	}
	
	// rozmiar pliku
	fseek(disk_file, 0, SEEK_END);
	disk_file_size = ftell(disk_file);
	rewind(disk_file);
	
	// sprawdzenie czy rozmiar pliku pokrywa sie z przypuszczanym rozmiarem na podstawie liczby blokow w superbloku
	if(disk_file_size != PREDICTED_VFS_FILE_SIZE(superblock.n_datablocks))
	{
		return 2;
	}

	// odczyt superbloku
	fseek(disk_file, SUPERBLOCK_OFFSET, SEEK_SET);
	if(fread(&superblock, sizeof(struct VFS_superblock), 1, disk_file) != 1)
	{
		return 3;
	}
	
	// odczyt katalogu 
	fseek(disk_file, ROOTDIR_BYTE_OFFSET, SEEK_SET);
	if(fread(&rootdir, sizeof(rootdir), 1, disk_file) != 1)
	{
		return 4;
	}
	
	// przygotowanie i odczyt bitmapy
	if(fu_map)
	{
		free_space_map = (bitmap)malloc(BITMAP_SIZE_ACTUAL_BYTES(superblock.n_datablocks));
		fseek(disk_file, BITMAP_BYTE_OFFSET, SEEK_SET);
		if(fread(free_space_map, BITMAP_SIZE_ACTUAL_BYTES(superblock.n_datablocks), 1, disk_file) != 1)
		{
			free(free_space_map);
			return 5;
		}
	}
	
	//przypisanie do przeslanych wskaznikow
	*sblock = superblock;
	*rdir = rootdir;
	if(fu_map) *fu_map = free_space_map;
	*fd = disk_file;
	if(d_fs) *d_fs = disk_file_size;
	
	return 0;
}


/*
 *	Funkcja sluzaca do wypisywania listy plikow w katalogu glownym VFS, poza tym wyswietla podstawowe 
 *	informacje o dysku takie jak rozmiar dysku, bloku itp. Dla kazdego pliku podana jest informacja o 
 *	rozmiarze faktycznym oraz o zajmowanym obszarze na dysku.
 */
int list_VFS_rootdir(char* filename)
{
	FILE* disk_file;
	struct VFS_superblock superblock;
	struct VFS_rootdir rootdir;
	bitmap free_space_bitmap;
	size_type disk_file_size, space_occupied = 0, space_occupied_total = 0, space_available_total = 0, space_free = 0;
	unsigned int i;
	int has_files_flag = 0;
	int r = open_and_read_VFS(filename, &superblock, &rootdir, NULL, &disk_file, &disk_file_size);
	if(r != 0) return r;
	
	printf("================================\nVFS successfully opened.\n================================\n\n");
	printf("Block size: %d\n", BLOCK_SIZE);
	printf("User data block size: %d\n", FILE_DATA_CHUNK_SIZE);
	printf("VFS file size: %dKB\n", disk_file_size/1024);
	printf("Nr of data blocks: %d\n", superblock.n_datablocks);
	
	printf("\n--------Root directory listing:--------\n");
	
	for(i = 0; i < MAX_FILECOUNT; ++i)
	{
		if(rootdir.files[i].filename[0] != 0)
		{
			space_occupied = FILE_CHUNKS_NEEDED_FOR_BYTES(rootdir.files[i].size)*FILE_DATA_CHUNK_SIZE; // pamiec zajmowana przez plik
			space_occupied_total += space_occupied; // pamiec ogolnie zajeta
			has_files_flag = 1;
			printf("File: %s, ", rootdir.files[i].filename);
			printf("real size: %d bytes, ",  rootdir.files[i].size);
			printf("size on disk: %dKB\n", space_occupied/1024);
		}
	}
	
	space_available_total = USABLE_DATA_SIZE_BYTES(superblock.n_datablocks);
	space_free = (space_available_total - space_occupied_total);
	
	printf("\nTotal usable disk space: %dKB\n", space_available_total/1024);
	printf("Used disk space: %dKB (%.2f\%)\n", space_occupied_total/1024, (space_occupied_total*100.0)/space_available_total);
	printf("Free disk space: %dKB (%.2f\%)\n", space_free/1024, (space_free*100.0)/space_available_total);
	
	if(!has_files_flag)
	{
		printf("No files.\n");
	}
	
	fclose(disk_file);
	return 0;
}


/*
 *	funkcja wyswietlajaca informacje o dysku: rozmiar bloku, rozmiar uzytecznej przestrzeni bloku, rozmiar pliku z VFS
 *	rozmiar uzytecznej pamieci VFS, ilosc blokow
 */
int read_VFS_info(char* filename)
{
	FILE* disk_file;
	struct VFS_superblock superblock;
	struct VFS_rootdir rootdir;
	bitmap free_space_bitmap;
	size_type disk_file_size, tmp_offset;
	int r;
	
	//pobranie info o dysku
	r = open_and_read_VFS(filename, &superblock, &rootdir, &free_space_bitmap, &disk_file, &disk_file_size);
	if(r != 0) return r;
	
	printf("================================\nVFS successfully opened.\n================================\n\n");
	printf("Block size: %d\n", BLOCK_SIZE);
	printf("User data block size: %d\n", FILE_DATA_CHUNK_SIZE);
	printf("VFS file size: %dKB\n", disk_file_size/1024);
	printf("Usable disk size: %dKB\n", USABLE_DATA_SIZE_BYTES(superblock.n_datablocks)/1024);
	printf("Nr of data blocks: %d\n", superblock.n_datablocks);
	printf("\nDisk structure:\n");
	printf("---------------------------------------------\n");
	printf("addr\tsize (blocks)\t        type\n");
	printf("---------------------------------------------\n");
	printf("%d\t%12d\tsuperblock\n", SUPERBLOCK_OFFSET, 1);
	printf("%d\t%12d\troot directory\n", ROOTDIR_BLOCK_OFFSET, ROOTDIR_SIZE_BLOCKS);
	printf("%d\t%12d\tspace usage bitmap\n", BITMAP_BLOCK_OFFSET, BITMAP_SIZE_BLOCKS(superblock.n_datablocks));
	
	//wyswietlanie informacji o zajetych/wolnych blokach
	iterate_block_structure(&is_free_bitmap_block, free_space_bitmap, superblock.n_datablocks, DATA_BLOCK_OFFSET(superblock.n_datablocks), "data");
	
	free(free_space_bitmap);
	fclose(disk_file);
	return 0;
	
}
	
