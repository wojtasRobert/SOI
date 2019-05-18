#include "VFS.h"
#include "helpers.h"

// funkcja zwracajaca info czy blok jest wolny/zajety zgodnie z podana mapa
int is_free_bitmap_block(int i, void* block_usage_map)
{
	bitmap b = (bitmap)block_usage_map;
	return !(BITTEST(b, i));
}

//funkcja wyswietlajaca adres i rozmiar kazdej z grup
void print_addr_size_type(size_type addr, size_type size, int is_free, char* type)
{
	printf("%d\t%12d\t", addr, size);
	if(is_free)
	{
		printf("free %s blocks\n", type);
	}
	else
	{
		printf("occupied %s blocks\n", type);
	}
}

/*
 * 	funkcja zliczajaca ilosc wolnych/zajetych blokow na podstawie przeslanej mapy zajetosci VFS,
 * 	grupuje bloki tego samego rodzaju i wyswietla adres i rozmiar kazdej z grup
 */
size_type iterate_block_structure(int (*is_free_condition)(int, void*), void* structure, size_type max_loop, size_type addr_offset, char* block_type)
{
	char is_free_space = 1;
	char is_free_block = 1;
	size_type i = 0;
	size_type curr_block_size = 0;
	size_type curr_block_addr = 0;
	
	// pierwszy blok
	is_free_block = (*is_free_condition)(0, structure);
	is_free_space = is_free_block;
	curr_block_addr = addr_offset+i;
	curr_block_size = 1;
	
	//petla grupujaca bloki wolne i zajete		
	for(i = 1; i < max_loop; ++i)
	{
		is_free_block = (*is_free_condition)(i, structure);
		if(is_free_block == is_free_space)
		{
			curr_block_size++;
		}
		else
		{
			print_addr_size_type(curr_block_addr, curr_block_size, is_free_space, block_type); // nowa grupa blokow wiec trzeba wypisac info o tej poprzedniej
			// poczatek drugiej grupy
			is_free_space = is_free_block;
			curr_block_addr = addr_offset+i;	// adres drugiej grupy
			curr_block_size = 1;
		}
	}
	
	print_addr_size_type(curr_block_addr, curr_block_size, is_free_block, block_type); // wypisanie info o kolejnej grupie
	return curr_block_addr + curr_block_size;
}
