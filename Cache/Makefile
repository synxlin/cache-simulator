
# Generic cache simulator Makefile
PROG = sim_cache
INCLS = -I.
SRCS = main.c cache.c op.c rbtree.c
OBJS = $(SRCS:.c=.o)
CLEANFILES = $(PROG) $(OBJS)


# Command line options
# The debug code can be truned on using the following variable from
# the command line while invoking make.
#
# Use "make DEBUG="-g -pg -D DBG_ON"" to have debug code eneabled for
# the current build. The debug build has:
# 	1. Debug symbils (-g switch in command line)
# 	2. Program profile data collector
# 	3. Enable debug traces in teh actual progran code.
#
DEBUG = 


# Compiler options
CC = gcc
OPTIMIZER = -O3
CFLAGS = -Wall -g -c $(DEBUG) $(OPTIMIZER) $(INCLS)
LFLAGS = -Wall -g $(DEBUG) $(OPTIMIZER) $(INCLS)

 
# Make directives
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	\rm -f $(CLEANFILES)

