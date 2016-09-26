#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

uint32_t NUM_LEVEL;
uint32_t BLOCKSIZE, REPL_POLICY, BLOCK_OFFSET_WIDTH;
char *TRACE_FILE;
uint64_t trace_count;

cache *CACHE;

uint64_t *OPTIMAL_TRACE;

#ifdef DBG
FILE *debug_fp;
#endif

int main(int argc, char* argv[])
{
#ifdef FLAG
	/*
	 *	originally design for N-level Cache
	 *	input should be (program) <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L1/L2_INCLUSION> <L2_SIZE> <L2_ASSOC> <L2/L3_INCLUSION> ...<LN_SIZE> <LN_ASSOC> <REPL_POLICY> <TRACE_FILE>
	 *	assume <LN/MAIN_MEMORY_INCLUSION> = NON_INCLUSIVE
	 */
	if ((argc + 1 - 4) % 3 != 0)
	{
		printf("Usage: %s <BLOCKSIZE> <L1_SIZE> <L1_ASSOC>  <L1/L2_INCLUSION>  <L2_SIZE> <L2_ASSOC> <L2/L3_INCLUSION> ... <LN_SIZE> <LN_ASSOC> <REPL_POLICY> <TRACE_FILE>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
#else
	if (argc != 9)
	{
		printf("Usage: %s <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPL_POLICY> <INCLUSION> <TRACE_FILE>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
#endif

#ifdef DBG
	debug_fp = fopen("debug.txt", "w");
	if (debug_fp == NULL)
		_error_exit("fopen")
#endif

	NUM_LEVEL = ((atoi(argv[4])) == 0) ? 1 : 2;

#ifdef FLAG
	NUM_LEVEL = (argc + 1 - 4) / 3;
#endif

	/* initial to parse arugments */
	uint32_t *size, *assoc, *inclusion;
	size = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (size == NULL)
		_error_exit("malloc")
	assoc = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (assoc == NULL)
		_error_exit("malloc")
	inclusion = (uint32_t *)malloc(sizeof(uint32_t) * NUM_LEVEL);
	if (inclusion == NULL)
		_error_exit("malloc")

	parse_arguments(argc, argv, size, assoc, inclusion);

	/* if it is optimal replace policy */
	/* we need to pre read the trace file to inital rank array */
	if (REPL_POLICY == OPTIMAL)
		OPTIMAL_TRACE_Initial();

	/* inital the cache */
	Cache_Initial(size, assoc, inclusion);

	free(size);
	free(assoc);
	free(inclusion);

	/* open trace file to read */
	FILE *trace_file_fp = fopen(TRACE_FILE, "r");
	if (trace_file_fp == NULL)
		_error_exit("fopen")

	while (1)
	{
#ifdef DBG
		fprintf(debug_fp, "--------------\n\n");
#endif
		int result;
		uint8_t OP, line;
		uint64_t ADDR;
		result = fscanf(trace_file_fp, "%c %llx%c", &OP, &ADDR, &line);
		trace_count++;
		uint64_t rank_value = (REPL_POLICY == OPTIMAL) ? OPTIMAL_TRACE[trace_count - 1] : trace_count;
		if (result == EOF)
			/* if we reach the end of trace file*/
			break;
		switch (OP)
		{
		case OP_READ:
		{
			block *blk = (block *)malloc(sizeof(block));
			Read(L1, ADDR, blk, rank_value);
			free(blk);
			break; 
		}
		case OP_WRITE:
			Write(L1, ADDR, DIRTY, rank_value);
			break;
		default:
			_input_error_exit("error: wrong operation type. Legal operations are read 'r' and write 'w'.\n")
			break;
		}
	}
	fclose(trace_file_fp);
	/* output the statistic result */
	output(stdout);

#ifdef DBG
	fprintf(debug_fp, "\n-----------------\n");
	int i, j, k;
	for (i = 0; i < NUM_LEVEL; i++)
	{
		for (j = 0; j < CACHE[i].CACHE_ATTRIBUTES.SET_NUM; j++)
		{
			for(k = 0; k < CACHE[i].CACHE_ATTRIBUTES.ASSOC; k++)
				fprintf(debug_fp, "%llx\t", Rebuild_Address(i, CACHE[i].SET[j].BLOCK[k].TAG, j));
			fprintf(debug_fp, "\n");
			for(k = 0; k < CACHE[i].CACHE_ATTRIBUTES.ASSOC; k++)
				fprintf(debug_fp, "%s\t", CACHE[i].SET[j].BLOCK[k].DIRTY_BIT ? "ditry" : "clean");
			fprintf(debug_fp, "\n");
		}
		fprintf(debug_fp, "\n");
	}
	fclose(debug_fp);
#endif

	if (REPL_POLICY == OPTIMAL)
		free(OPTIMAL_TRACE);
	Cache_free();
	return 0;
}
