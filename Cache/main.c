#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

uint32_t NUM_LEVEL;
uint32_t BLOCKSIZE, BLOCK_OFFSET_WIDTH;
char *TRACE_FILE;
uint64_t trace_count;

cache *CACHE;

uint64_t *OPTIMIZATION_TRACE;

#ifdef DBG
FILE *debug_fp;
#endif

int main(int argc, char* argv[])
{
	if (argc != 9)
	{
		printf("Usage: %s <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPL_POLICY> <INCLUSION> <TRACE_FILE>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	/*
	 *	originally design for N-level Cache
	 *	input should be (program) <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L1_REPL_POLICY> <L1/L2_INCLUSION> <L2_SIZE> <L2_ASSOC> <L2_REPL_POLICY> <L2/L3_INCLUSION> ...<LN_REPL_POLICY> <TRACE_FILE>
	 *	assume <LN/MAIN_MEMORY_INCLUSION> = NON_INCLUSIVE
	 *	NUM_LEVEL = (argc + 1 - 3) / 4;
	 */
	NUM_LEVEL = ((atoi(argv[4])) == 0) ? 1 : 2;

	uint32_t *size, *assoc, *repl_policy, *inclusion;
	size = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (size == NULL)
		_error_exit("malloc")
	assoc = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (assoc == NULL)
		_error_exit("malloc")
	repl_policy = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (repl_policy == NULL)
		_error_exit("malloc")
	inclusion = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (inclusion == NULL)
		_error_exit("malloc")

	BLOCKSIZE = atoi(argv[1]);
	size[L1] = atoi(argv[2]);
	assoc[L1] = atoi(argv[3]);
	size[L2] = atoi(argv[4]);
	assoc[L2] = atoi(argv[5]);
	repl_policy[L2] = repl_policy[L1] = atoi(argv[6]);
	inclusion[L1] = atoi(argv[7]);
	inclusion[L2] = NON_INCLUSIVE;
	TRACE_FILE = argv[8];

	uint8_t flag = input_check(size, assoc, repl_policy, inclusion);

	if (flag == 1)
		OPTIMIZATION_TRACE_Initial();

	Cache_Initial(size, assoc, repl_policy, inclusion);

	FILE *trace_file_fp = fopen(TRACE_FILE, "r");
	if (trace_file_fp == NULL)
		_error_exit("fopen")
#ifdef DBG
	debug_fp = fopen("debug.txt", "w");
	if (debug_fp == NULL)
		_error_exit("fopen")
#endif

	while (1)
	{
		int result;
		uint8_t OP, line;
		uint64_t ADDR;
		result = fscanf(trace_file_fp, "%c %llx%c", &OP, &ADDR, &line);
		trace_count++;
		if (result == EOF)
			break;
		switch (OP)
		{
		case READ:
		{
			block *blk = (block *)malloc(sizeof(block));
			Read(L1, ADDR, blk, 0);
			free(blk);
			break; 
		}
		case WRITE:
			Write(L1, ADDR, DIRTY);
			break;
		default:
			_input_error_exit("error: wrong operation type. Legal operations are read 'r' and write 'w'.\n")
			break;
		}
	}
	fclose(trace_file_fp);

	file_output();

	if (flag == 1)
		free(OPTIMIZATION_TRACE);
	Cache_free();
}
