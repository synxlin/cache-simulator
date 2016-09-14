#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

void Cache_Initial(uint32_t *size, uint32_t *assoc, uint32_t *repl_policy, uint32_t *inclusion)
{
	int j;
	CACHE = (cache *)malloc(sizeof(cache)*NUM_LEVEL);
	BLOCK_OFFSET_WIDTH = log_2(BLOCKSIZE);
	for (j = 0; j < NUM_LEVEL; j++)
	{
		if (size[j] == 0)
			break;
		CACHE[j].SIZE = size[j];
		CACHE[j].ASSOC = assoc[j];
		CACHE[j].REPL_POLICY = repl_policy[j];
		CACHE[j].INCLUSION = inclusion[j];
		CACHE[j].SET_NUM = CACHE[j].SIZE / (CACHE[j].ASSOC * BLOCKSIZE);
		
		CACHE[j].INDEX_WIDTH = log_2(CACHE[j].SET_NUM);
		CACHE[j].TAG_WIDTH = 32 - CACHE[j].INDEX_WIDTH - BLOCK_OFFSET_WIDTH;

		CACHE[j].CACHE_IC = (set *)malloc(sizeof(set) * CACHE[j].SET_NUM);

		uint32_t i;
		for (i = 0; i < CACHE[j].SET_NUM; i++)
		{
			CACHE[j].CACHE_IC[i].BLOCK = (block *)malloc(sizeof(block) * CACHE[j].ASSOC);
			CACHE[j].CACHE_IC[i].RANK = (uint64_t *)malloc(sizeof(uint64_t) * CACHE[j].ASSOC);
		}
	}
}

void Interpret_Address(uint32_t level, uint64_t ADDR, uint64_t *tag, uint64_t *index)
{
	uint32_t tag_width = CACHE[level].TAG_WIDTH;
	*tag = ADDR >> (32 - tag_width);
	*index = (ADDR << tag_width) >> (tag_width + BLOCK_OFFSET_WIDTH);
}

uint64_t Rebuild_Address(uint8_t level, uint64_t tag, uint64_t index)
{
	uint64_t ADDR;
	ADDR |= (tag << (CACHE[level].INDEX_WIDTH + BLOCK_OFFSET_WIDTH));
	ADDR |= (index << BLOCK_OFFSET_WIDTH);
	return ADDR;
}

uint8_t Cache_Search(uint8_t level, uint64_t tag, uint64_t index, uint32_t *way_num)
{
	int i, k = -1;
	for (i = 0; i < CACHE[level].ASSOC; i++)
		if (CACHE[level].CACHE_IC[index].BLOCK[i].TAG == tag &&  CACHE[level].CACHE_IC[index].BLOCK[i].VALID_BIT == VALID)
		{
			k = i;
			break;
		}
	if (k == -1)
		return MISS;
	else
	{
		*way_num = k;
		return HIT;
	}
}

void OPTIMIZATION_TRACE_Initial()
{
	FILE *trace_file_fp = fopen(TRACE_FILE, "r");
	if (trace_file_fp == NULL)
		error_exit("fopen()")
	uint64_t trace_len = 1000, i = 0;
	uint64_t *trace = (uint64_t *)malloc(sizeof(uint64_t) * trace_len);
	while (1)
	{
		int result;
		uint8_t OP;
		uint64_t ADDR;
		result = fscanf(trace_file_fp, "%c %x", &OP, &ADDR);
		if (result == EOF)
			break;
		if (i > trace_len)
		{
			trace_len *= 2;
			trace = (uint32_t *)realloc(OPTIMIZATION_TRACE, trace_len);
		}
		trace[i] = ADDR;
		i++;
	}
	fclose(trace_file_fp);

	OPTIMIZATION_TRACE = (uint64_t *)malloc(sizeof(uint64_t) * i);

	_rb_tree_ptr T = (_rb_tree_ptr)malloc(sizeof(struct _rb_tree));
	T->_root = NULL;
	uint64_t j;
	for (j = i - 1; j >= 0; j--)
	{
		_rb_tree_node_ptr _result = _rb_tree_find(T, trace[j]);
		if (_result == NULL)
		{
			_rb_tree_insert(T, trace[j], j);
			OPTIMIZATION_TRACE[j] = i + 1;
		}
		else
		{
			OPTIMIZATION_TRACE[j] = _result->value - j;
			_result->value = j;
		}
	}
	free(trace);
	_rb_tree_clear(T);
}

void Rank_Maintain(uint8_t level, uint64_t index, uint32_t way_num, uint8_t result)
{
	uint64_t *rank = CACHE[level].CACHE_IC[index].RANK;
	switch (CACHE[level].REPL_POLICY)
	{
	case FIFO:
		if (result == MISS)
			rank[way_num] = trace_count;
		return; 
	case PLRU:
	{
		if (CACHE[level].ASSOC == 1)
			return;
		uint32_t idx = CACHE[level].ASSOC - 1 + way_num;
		uint8_t l_or_r = LEFT;
		l_or_r = (idx & 1) ? LEFT : RIGHT;
		idx = ((idx + 1) >> 1) - 1;
		while (idx >= 0)
		{
			rank[idx] = 1 - l_or_r;
			l_or_r = idx & 1;
			idx = ((idx + 1) >> 1) - 1;
		}
		return;
	}
	case OPTIMIZATION:
		rank[way_num] = OPTIMIZATION_TRACE[trace_count];
		return;
	case LRU:
		rank[way_num] = trace_count;
		return;
	}
}

uint32_t Rank_Top(uint8_t level, const uint64_t *rank)
{
	switch (CACHE[level].REPL_POLICY)
	{
	default:
	case FIFO:
	case LRU:
	{
		uint32_t i, k = 0;
		for (i = 0; i < CACHE[level].ASSOC; i++)
			if (rank[i] < rank[k])
				k = i;
		return k;
	}
	case PLRU:
	{
		uint32_t assoc = CACHE[level].ASSOC;
		if (assoc == 1)
			return 0;
		uint32_t idx = 0;
		while (idx < 2 * assoc - 1)
		{
			if (rank[idx] == LEFT)
				idx = (idx << 1) + 1;
			else
				idx = (idx << 1) + 2;
		}
		idx = idx - assoc + 1;
		return idx;
	}
	case OPTIMIZATION:
	{
		uint32_t i, k = 0;
		for (i = 0; i < CACHE[level].ASSOC; i++)
			if (rank[i] > rank[k])
				k = i;
		return k;
	}
	}
}

uint32_t Cache_Replacement(uint8_t level, uint64_t index, uint64_t tag)
{
	uint64_t *rank = CACHE[level].CACHE_IC[index].RANK;
	uint32_t way_num = Rank_Top(level, rank);
	block *tmp = &(CACHE[level].CACHE_IC[index].BLOCK[way_num]);
	if (tmp->VALID_BIT == INVALID)
	{
		tmp->VALID_BIT = VALID;
		tmp->TAG = tag;
#ifdef DBG
		printf("create new in L%1d\n", level+1);
#endif // DBG
	}
	else
	{
		uint64_t ADDR = Rebuild_Address(level, tmp->TAG, index);
#ifdef DBG
		printf("replace old %x in L%1d - write back\n", ADDR, level+1);
		printf("replace new\n");
#endif // DBG	
		Write_Back(level, ADDR);

		if (tmp->DIRTY_BIT == DIRTY)
			tmp->DIRTY_BIT = CLEAN;
		tmp->TAG = tag;
	}
	return way_num;
}

void Write_Back(uint8_t level, uint64_t ADDR)
{
	switch (CACHE[level].INCLUSION)
	{
	case NON_INCLUSIVE:
	case INCLUSIVE:
	{
		uint32_t nxt_level = level + 1;
		if (nxt_level >= NUM_LEVEL)
		{
			Write(NUM_LEVEL, ADDR);
			break;
		}
		uint64_t tag, index;
		uint32_t way_num;
		Interpret_Address(nxt_level, ADDR, &tag, &index);
		uint8_t result = Cache_Search(nxt_level, tag, index, &way_num);
		if (result == HIT) // HIT说明新的没有和旧的重复, MISS说明已经被新的替换了, 需要从下级写回下下级
			CACHE[nxt_level].CACHE_IC[index].BLOCK[way_num].DIRTY_BIT = DIRTY;
		else
			Write_Back(nxt_level, ADDR);
		break;
	}
	case EXCLUSIVE:
		Write(level + 1, ADDR);
		break;
	}
	
	Invalidation(level - 1, ADDR);
}

void Invalidation(uint8_t level, uint64_t ADDR)
{
	if (level < L1)
		return;
	switch (CACHE[level].INCLUSION)
	{
	case INCLUSIVE:
	{
		uint64_t tag, index;
		uint32_t way_num;
		Interpret_Address(level, ADDR, &tag, &index);
		uint8_t result = Cache_Search(level, tag, index, &way_num);
		if (result == HIT)
			CACHE[level].CACHE_IC[index].BLOCK[way_num].VALID_BIT = INVALID;
		Invalidation(level - 1, ADDR);
	}
	default:
		return;
	}
}

uint8_t Read(uint64_t ADDR)
{
	uint64_t *tag = (uint64_t *)malloc(sizeof(uint64_t)*NUM_LEVEL);
	uint64_t *index = (uint64_t *)malloc(sizeof(uint64_t)*NUM_LEVEL);
	uint32_t *way_num = (uint32_t *)malloc(sizeof(uint32_t)*NUM_LEVEL);
	Interpret_Address(L1, ADDR, &tag[L1], &index[L1]);
	uint8_t result = Cache_Search(L1, tag[L1], index[L1], &way_num[L1]);
	if (result == HIT)
	{
#ifdef DBG
		printf("read %x : L1 HIT\n", ADDR);
#endif // DEBUG
		Rank_Maintain(L1, index[L1], way_num[L1], HIT);
		return HIT;
	}
	else
	{
		uint32_t level = L2;
		while (level < NUM_LEVEL)
		{
			Interpret_Address(level, ADDR, &tag[level], &index[level]);
			result = Cache_Search(level, tag[level], index[level], &way_num[level]);
			if (result == HIT)
			{
#ifdef DBG
				printf("read %x : L1 MISS L2 HIT\n", ADDR);
#endif // DBG
				switch (INCLUSION)
				{
				case NON_INCLUSIVE:
				case INCLUSIVE:
				{
					uint32_t pre_level = level - 1;
					while (pre_level >= L1)
					{
						way_num[pre_level] = Cache_Replacement(pre_level, index[pre_level], tag[pre_level]);
						Rank_Maintain(pre_level, index[pre_level], way_num[pre_level], MISS);
					}
					Rank_Maintain(level, index[level], way_num[level], HIT);
					return MISS;
				}
				case EXCLUSIVE:
					CACHE[L2][index[L2]].BLOCK[way_num[L2]].VALID_BIT = INVALID;
					way_num[L1] = Cache_Replacement(L1, index[L1], tag[L1]);
					Rank_Maintain(L1, index[L1], way_num[L1], MISS);
					return MISS;
				}
			}
			else
			{
#ifdef DBG
				printf("read %x : L1 MISS L2 MISS\n", ADDR);
#endif // DBG
				switch (INCLUSION)
				{
				case INCLUSIVE:
					Cache_Replacement(L2, index[L2], tag[L2]);
					Cache_Replacement(L1, index[L1], tag[L1]);
					return MISS;
				case EXCLUSIVE:
					Cache_Replacement(L1, index[L1], tag[L1]);
					return MISS;
				}
			}
		}
		if (SIZE[level] == 0)
		{
#ifdef DBG
			printf("read %x : L1 MISS\n", ADDR);
#endif // DEBUG
			way_num[L1] = Cache_Replacement(L1, index[L1], tag[L1]);
			Rank_Maintain(L1, index[L1], way_num[L1], MISS);
			return MISS;
		}
	}
}

void Write(uint8_t level, uint64_t ADDR)
{
	uint64_t tag[2], index[2];
	uint32_t way_num[2];
	if (level == L1)
	{
		uint8_t result = Read(ADDR, tag, index, way_num);
		if (result == HIT)
		{
#ifdef DBG
			printf("write %x : L1 HIT\n", ADDR);
#endif // DBG
			;
		}
		else
		{
#ifdef DBG
			printf("write %x : L1 MISS\n", ADDR);
#endif // DBG
			;
		}
	}
	else
	{
		uint64_t tag, index;
		uint32_t way_num;
		Interpret_Address(level, ADDR, &tag, &index);
		uint8_t result = Cache_Search(level, tag, index, &way_num);
		if (result == HIT)
		{
#ifdef DBG
			printf("write %x : L2 HIT\n", ADDR);
#endif // DBG
			;
		}
		else
		{
			way_num = Cache_Replacement(L2, index, tag);
			;
#ifdef DBG
			printf("write %x : L2 MISS\n", ADDR);
#endif // DBG
		}
	}
	CACHE[level][index[level]].BLOCK[way_num[level]].DIRTY_BIT = DIRTY;
}

