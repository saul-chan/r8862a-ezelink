#ifndef _MEMLOAD_H
#define _MEMLOAD_H

#define LITTLE_ENDIAN_MODE 0
#define BIG_ENDIAN_MODE 1
#define MMAP_SIZE 4096

#define FILE_OPTION_SIZE 4

enum format_list
{
	FORMAT_HEX32,
	FORMAT_BIN32
};

#endif
