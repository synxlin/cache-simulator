
# Generic cache simulator Makefile
PROG = sim_cache
SRCS = main.c cache.c op.c rbtree.c
OBJS = $(SRCS:.c=.o)
CLEANFILES = $(PROG) $(OBJS)

# input parameter
# for debug, input $make FLAG="-g -D DBG"
# for complete function, input $make FLAG="-D FLAG"
FLAG = 

# Compiler options
CC = gcc
OPTIMIZER = -O3
CFLAGS = -Wall -c $(FLAG) $(OPTIMIZER)
LFLAGS = -Wall $(FLAG) $(OPTIMIZER)

# default rule
all: $(PROG)
	@echo "my work is done here..."

# rule for making sim_cache
$(PROG): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@
	@echo "-----------DONE WITH SIM_CACHE-----------"

# generic rule for converting any .c file to any .o file
.c.o:
	$(CC) $(CFLAGS) $< -o $@

# type "make clean" to remove all .o files plus the sim_cache binary
clean:
	$(RM) $(CLEANFILES)

# type "make clobber" to remove all .o files (leaves sim_cache binary)
clobber:
	$(RM) $(OBJS)
