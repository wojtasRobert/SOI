#ifndef SOI_VFS_TYPES_H_
#define SOI_VFS_TYPES_H_

#include <stdint.h>
#include <limits.h>		

//operacje na bitach mapy mitowej
#define BITMASK(b) (1 << ((b) % CHAR_BIT))		// maska bitu
#define BITSLOT(b) ((b) / CHAR_BIT)			// slot na bit w mapie 
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b)) 	// ustawienie bitu
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b)) // reset bitu
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))	// sprawdzenie wartosci bitu	
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)	// rozmiar mapy w bajtach

typedef uint32_t size_type; 		// 4 bajty - sluzy do adresowania i wyrazania rozmiaru (w blokach)
typedef uint32_t block_index_type; 	// 4 bajty
typedef char* bitmap;			// wsk na mape bitowa

#endif
