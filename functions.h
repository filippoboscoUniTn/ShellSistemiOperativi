//include guard
#ifndef FUNCTIONS__H_
#define FUNCTIONS__H_





#include "types.h"

//prints usage text
void printUsage( char* _prog_name );
//gets parameter's code
argcode_t getcode(char *_arg);
//gets parameter's type
argtype_t gettype(argcode_t _argcode);
//removes argv[0] from arguments matrix
void catch_args(int argc, char **argv, char **args);
//links two file descriptors
void link_pipe(int _source, int _destination);
//allocates space for holding strings
char* my_malloc(int _n);





#endif
