#ifndef OP_H_
#define OP_H_

#define error_exit(fun) { perror(fun); exit(EXIT_FAILURE); }
#define input_error_exit(fun) { printf(fun); exit(EXIT_FAILURE);}

uint32_t log_2(uint32_t num);

#endif
