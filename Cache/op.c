#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

char *NAME_REPL_POLICY[] = { "LRU","FIFO","Pseudo","Optimization" };
char *NAME_INCLUSION[] = { "non-inclusive", "inclusive", "exclusive" };

uint32_t log_2(uint32_t num)
{
	uint32_t result = 0, tmp = num;
	while (tmp >>= 1)
		result++;
	return result;
}

uint8_t input_check(uint32_t *size, uint32_t *assoc, uint32_t *repl_policy, uint32_t *inclusion)
{
	uint32_t i;
	uint8_t optimizatoin_flag = 0;
	for (i = 0; i < NUM_LEVEL; i++)
	{
		if(!_is_power_of_2(size[i]))
			_input_error_exit("error: wrong cache size")
		if(!_is_power_of_2(assoc[i]))
			_input_error_exit("error: wrong associatity")
		if(repl_policy[i] < LRU || repl_policy[i] > OPTIMIZATION)
			_input_error_exit("error: wrong replacement policy")
		if(inclusion[i] < NON_INCLUSIVE || inclusion[i] > EXCLUSIVE)
			_input_error_exit("error: wrong inclusion")
		if (repl_policy[i] == OPTIMIZATION)
			optimizatoin_flag = 1;
	}
	return optimizatoin_flag;
}

void file_output()
{
	FILE *fp = fopen("RESULT.txt", "w");
	if (fp == NULL)
		_error_exit("fopen")

	fprintf(fp, "===== Simulator configuration =====\n");
	fprintf(fp, "BLOCKSIZE:             %u\n", BLOCKSIZE);
	fprintf(fp, "L1_SIZE:               %u\n", CACHE[L1].SIZE);
	fprintf(fp, "L1_ASSOC:              %u\n", CACHE[L1].ASSOC);
	fprintf(fp, "L2_SIZE:               %u\n", CACHE[L2].SIZE);
	uint64_t num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].ASSOC;
	fprintf(fp, "L2_ASSOC:              %u\n", num_L2);
	fprintf(fp, "REPLACEMENT POLICY:    %s\n", NAME_REPL_POLICY[CACHE[L1].REPL_POLICY]);
	fprintf(fp, "INCLUSION PROPERTY:    %s\n", NAME_INCLUSION[CACHE[L1].INCLUSION]);
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
	num_L2 = (NUM_LEVEL == 1) ? 0 : CACHE[L2].CACHE_STAT.num_read_misses;
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

	fclose(fp);
}