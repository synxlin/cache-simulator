#pragma once
#ifndef OP_H_
#define OP_H_

#define _error_exit(fun) { perror(fun); exit(EXIT_FAILURE); }

#define _input_error_exit(fun) { printf(fun); exit(EXIT_FAILURE);}

#define _is_power_of_2(x) (((x) == 0) || ((x) > 0 && !((x) & ((x) - 1))))

extern char *NAME_REPL_POLICY[];
extern char *NAME_INCLUSION[];

uint32_t log_2(uint32_t num);

void parse_arguments(int argc, char* argv[], uint32_t *size, uint32_t *assoc, uint32_t *inclusion);

void output(FILE *fp);

#endif
