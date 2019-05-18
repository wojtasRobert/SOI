#ifndef SOI_VFS_HELPERS_H_
#define SOI_VFS_HELPERS_H_
#include "VFS.h"
#include <stdio.h>

int is_free_bitmap_block(int i, void* block_usage_map);

void print_addr_size_type(size_type addr, size_type size, int is_free, char* type);

size_type iterate_block_structure(int (*is_free_condition)(int, void*), void* structure, size_type max_loop, size_type addr_offset, char* block_type);
#endif
