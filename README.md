# Cache Hierarchy Simulator

## Introduction 

This is a cache hierarchy simulator.

**NOTE**: optimal replacement implement has bugs.

## Usage

Using the command shown as follows.

To complie the program,

>make

To run the program,

>sim_cache < BLOCKSIZE \> < L1_SIZE \> < L1_ASSOC \> < L2_SIZE \> < L2_ASSOC \> < REPL_POLICY \> < INCLUSION \> < TRACE_FILE \> 

### Input

|	Parameters		|	Introduction																|
|-------------------|:-----------------------------------------------:								|
|	BLOCKSIZE:   	|Positive int. Block size in bytes (assumed to be same for all caches) 			|
|	L1_SIZE:    	|Positive int. L1 cache size in bytes	  										|
|	L1_ASSOC:    	|Positive int. L1 set‐associativity (1 is direct‐mapped)						|
|	L2_SIZE:    	|Positive int. L2 cache size in bytes; 0 signifies that there is no L2 cache	|
|	L2_ASSOC:    	|Positive int. L2 set‐associativity (1 is direct‐mapped)						|
|	REPL_POLICY:  	|Positive int. 0 for LRU, 1 for FIFO, 2 for pseudoLRU, 3 for optimal			|
|	INCLUSION:  	|Positive int. 0 for non‐inclusive, 1 for inclusive and 2 for exclusive		|
|	TRACE_FILE:  	|Character string. Full name of trace file including any extensions				|

### Output

1. Memory hierarchy configuration and trace filename. 

2. The following measurements: 

	a. Number of L1 reads

	b. Number of L1 read misses

	c. Number of L1 writes

	d. Number of L1 write misses

	e. L1 miss rate

		= (L1 read misses + L1 write misses)/(L1 reads + L1 writes) 

	f. Number of writebacks from L1

		dirty evictions from the L1 (with an Exclusive L2, clean L1 evictions are also written to L2 but don’t count those here)

	g. Number of L2 reads

		= L1 read misses + L1 write misses

	h. Number of L2 read misses

	i. Number of L2 writes

		= writebacks from L1, in case of an Inclusive or Non‐inclusive L2

		= writebacks from L1 + clean L1 evictions, in case of an Exclusive L2

	j. Number of L2 write misses

	k. L2 miss rate

		= (L2 read misses + L2 write misses)/(L2 reads + L2 writes)

	l. Number of writebacks from L2 to memory

	m. Total memory traffic (or the number of blocks transferred to/from memory)

		Assuming the presence of an L2, this should match L2 read misses + L2 write misses + writebacks from L2 to memory, in case of a Non‐inclusive or Exclusive L2 cache. In case of an Inclusive L2, if the blocks evicted from the L1 as a result of the back invalidation happen to be dirty, those should also be taken into account as writes to the memory (since they hold more recent data than what the memory contains).  

### Trace File

The simulator reads in a trace file in the following format:

r|w < hex address \>

r|w < hex address \>

...

The first argument is the operation. The character “r” means it is a read operation and the character “w” indicates a write operation. The second argument is the accessed address in hexadecimal format. 

Note that we assume the memory is byte addressable, so to obtain the block address you need to 
properly mask the provided address. The addresses can be up to 64 bits long. 

Here is a sample trace: 

r fff432
