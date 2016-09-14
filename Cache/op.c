#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

uint32_t log_2(uint32_t num)
{
	uint32_t result = 0, tmp = num;
	while (tmp >>= 1)
		result++;
	return result;
}

