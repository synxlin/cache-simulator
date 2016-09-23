#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

char *NAME_REPL_POLICY[] = { "LRU","FIFO","Pseudo","Optimization" };
char *NAME_INCLUSION[] = { "non-inclusive", "inclusive", "exclusive" };

/*
 *	improved log2 function
 *	log2(0) = 0
 */
uint32_t log_2(uint32_t num)
{
	uint32_t result = 0, tmp = num;
	while (tmp >>= 1)
		result++;
	return result;
}

/*
 *	parse arguments and check inputs
 */
void parse_arguments(int argc, char* argv[], uint32_t *size, uint32_t *assoc, uint32_t *inclusion)
{
	uint32_t i;
#ifdef FLAG
	BLOCKSIZE = atoi(argv[1]);
	for (i = 0; i < NUM_LEVEL; i++)
	{
		size[i] = atoi(argv[2 + i * 3]);
		assoc[i] = atoi(argv[3 + i * 3]);
		if (i != NUM_LEVEL - 1)
			inclusion[i] = atoi(argv[4 + i * 3]);
		else
			inclusion[i] = NON_INCLUSIVE;
	}
	REPL_POLICY = atoi(argv[argc - 2]);
	TRACE_FILE = argv[argc - 1];
#else
	BLOCKSIZE = atoi(argv[1]);
	size[L1] = atoi(argv[2]);
	assoc[L1] = atoi(argv[3]);
	inclusion[L1] = atoi(argv[7]);
	if (NUM_LEVEL == 2)
	{
		size[L2] = atoi(argv[4]);
		assoc[L2] = atoi(argv[5]);
		inclusion[L2] = NON_INCLUSIVE;
	}
	REPL_POLICY = atoi(argv[6]);
	TRACE_FILE = argv[8];
#endif
	/* input check */
	if(REPL_POLICY < LRU || REPL_POLICY > OPTIMIZATION)
			_input_error_exit("error: wrong replacement policy\n")
	for (i = 0; i < NUM_LEVEL; i++)
	{
		if(!_is_power_of_2(size[i]))
			_input_error_exit("error: wrong cache size\n")
		if(!_is_power_of_2(assoc[i]))
			_input_error_exit("error: wrong associatity\n")
		if(inclusion[i] < NON_INCLUSIVE || inclusion[i] > EXCLUSIVE)
			_input_error_exit("error: wrong inclusion\n")
#ifdef FLAG
		if(size[i] == 0)
			_input_error_exit("error: wrong cache size\n")
#endif
	}
	if(NUM_LEVEL == 1 && inclusion[L1] != NON_INCLUSIVE)
		_input_error_exit("error: wrong inclusion\n")
}

/*
 *	output cache statistic results to file or stdout
 */
void output(FILE *fp)
{
#ifdef FLAG
	fprintf(fp, "===== Simulator configuration =====\n");
	fprintf(fp, "BLOCKSIZE:                 %u\n", BLOCKSIZE);
	uint32_t i;
	for (i = 0; i < NUM_LEVEL; i++)
	{
		fprintf(fp, "L%u_SIZE:                   %u\n", i + 1, CACHE[i].CACHE_ATTRIBUTES.SIZE);
		fprintf(fp, "L%u_ASSOC:                  %u\n", i + 1, CACHE[i].CACHE_ATTRIBUTES.ASSOC);
		if (i < NUM_LEVEL - 1)
			fprintf(fp, "L%u/L%u INCLUSION PROPERTY:  %s\n", i + 1, i + 2, NAME_INCLUSION[CACHE[i].CACHE_ATTRIBUTES.INCLUSION]);
	}
	fprintf(fp, "REPLACEMENT POLICY:        %s\n", NAME_REPL_POLICY[REPL_POLICY]);
	fprintf(fp, "trace_file:                %s\n", TRACE_FILE);
	fprintf(fp, "===== Simulation results (raw) =====\n");
	for (i = 0; i < NUM_LEVEL; i++)
	{
		fprintf(fp, "%c. number of L%u reads:        %llu\n", 'a' + i * 6, i + 1, CACHE[i].CACHE_STAT.num_reads);
		fprintf(fp, "%c. number of L%u read misses:  %llu\n", 'b' + i * 6, i + 1, CACHE[i].CACHE_STAT.num_read_misses);
		fprintf(fp, "%c. number of L%u writes:       %llu\n", 'c' + i * 6, i + 1, CACHE[i].CACHE_STAT.num_writes);
		fprintf(fp, "%c. number of L%u write misses: %llu\n", 'd' + i * 6, i + 1, CACHE[i].CACHE_STAT.num_write_misses);
		double miss_rate = ((double)CACHE[i].CACHE_STAT.num_read_misses + (double)CACHE[i].CACHE_STAT.num_write_misses) / ((double)CACHE[i].CACHE_STAT.num_reads + (double)CACHE[i].CACHE_STAT.num_writes);
		fprintf(fp, "%c. L%u miss rate:              %f\n", 'e' + 6 * i, i + 1, miss_rate);
		fprintf(fp, "%c. number of L%u writebacks:   %llu\n", 'f' + 6 * i, i + 1, CACHE[i].CACHE_STAT.num_write_backs);
	}
	fprintf(fp, "%c. total memory traffic:      %llu\n", 'a' + i * 6, CACHE[NUM_LEVEL - 1].CACHE_STAT.num_blocks_transferred);
#else
	fprintf(fp, "===== Simulator configuration =====\n");
	fprintf(fp, "BLOCKSIZE:             %u\n", BLOCKSIZE);
	fprintf(fp, "L1_SIZE:               %u\n", CACHE[L1].CACHE_ATTRIBUTES.SIZE);
	fprintf(fp, "L1_ASSOC:              %u\n", CACHE[L1].CACHE_ATTRIBUTES.ASSOC);
	uint64_t num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_ATTRIBUTES.SIZE;
	fprintf(fp, "L2_SIZE:               %llu\n", num_L2);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_ATTRIBUTES.ASSOC;
	fprintf(fp, "L2_ASSOC:              %llu\n", num_L2);
	fprintf(fp, "REPLACEMENT POLICY:    %s\n", NAME_REPL_POLICY[REPL_POLICY]);
	fprintf(fp, "INCLUSION PROPERTY:    %s\n", NAME_INCLUSION[CACHE[L1].CACHE_ATTRIBUTES.INCLUSION]);
	fprintf(fp, "trace_file:            %s\n", TRACE_FILE);
	fprintf(fp, "===== Simulation results (raw) =====\n");
	fprintf(fp, "a. number of L1 reads:        %llu\n", CACHE[L1].CACHE_STAT.num_reads);
	fprintf(fp, "b. number of L1 read misses:  %llu\n", CACHE[L1].CACHE_STAT.num_read_misses);
	fprintf(fp, "c. number of L1 writes:       %llu\n", CACHE[L1].CACHE_STAT.num_writes);
	fprintf(fp, "d. number of L1 write misses: %llu\n", CACHE[L1].CACHE_STAT.num_write_misses);
	double miss_rate = ((double)CACHE[L1].CACHE_STAT.num_read_misses + (double)CACHE[L1].CACHE_STAT.num_write_misses) / ((double)CACHE[L1].CACHE_STAT.num_reads + (double)CACHE[L1].CACHE_STAT.num_writes);
	fprintf(fp, "e. L1 miss rate:              %f\n", miss_rate);
	fprintf(fp, "f. number of L1 writebacks:   %llu\n", CACHE[L1].CACHE_STAT.num_write_backs);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_reads;
	fprintf(fp, "g. number of L2 reads:        %llu\n", num_L2);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_read_misses;
	fprintf(fp, "h. number of L2 read misses:  %llu\n", num_L2);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_writes;
	fprintf(fp, "i. number of L2 writes:       %llu\n", num_L2);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_write_misses;
	fprintf(fp, "j. number of L2 write misses: %llu\n", num_L2);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_write_misses;
	miss_rate = ((double)CACHE[L2].CACHE_STAT.num_read_misses + (double)CACHE[L2].CACHE_STAT.num_write_misses) / ((double)CACHE[L2].CACHE_STAT.num_reads + (double)CACHE[L2].CACHE_STAT.num_writes);
	if(NUM_LEVEL == 1)
		fprintf(fp, "k. L2 miss rate:              %d\n", 0);
	else
		fprintf(fp, "k. L2 miss rate:              %f\n", miss_rate);
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_write_backs;
	fprintf(fp, "l. number of L2 writebacks:   %llu\n", num_L2);
	fprintf(fp, "m. total memory traffic:      %llu\n", CACHE[NUM_LEVEL - 1].CACHE_STAT.num_blocks_transferred);
#endif
}
