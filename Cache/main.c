#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

uint32_t NUM_LEVEL;
uint32_t BLOCKSIZE, BLOCK_OFFSET_WIDTH;
char *TRACE_FILE;
uint64_t trace_count;

set *CACHE[2];

uint64_t *OPTIMIZATION_TRACE;

int main(int argc, char* argv[])
{
	if (argc != 9)
	{
		printf("Usage: %s <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPL_POLICY> <INCLUSION> <TRACE_FILE>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	BLOCKSIZE = atoi(argv[1]);
	SIZE[L1] = atoi(argv[2]);
	ASSOC[L1] = atoi(argv[3]);
	SIZE[L2] = atoi(argv[4]);
	ASSOC[L2] = atoi(argv[5]);
	REPL_POLICY = atoi(argv[6]);
	INCLUSION = atoi(argv[7]);
	TRACE_FILE = argv[8];

	Cache_Initial();

	if (REPL_POLICY == OPTIMIZATION)
		OPTIMIZATION_TRACE_Initial();

	while (1)
	{
		char OP;
		uint32_t ADDR;
		scanf("%c %x", &OP, &ADDR);
		uint32_t tag[2], index[2], way_num[2];
		if (OP == READ)
			Read(ADDR, tag, index, way_num);
		else
			Write(L1, ADDR, tag, index, way_num);
	}
}
