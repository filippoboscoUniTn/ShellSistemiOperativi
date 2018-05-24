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
//gets environment variable key string identifier from code
char* get_envstring(argcode_t _argcode, char *_destination);
//removes argv[0] from arguments matrix
void catch_args(int argc, char **argv, char **args);
//links two file descriptors
void link_pipe(int _source, int _destination);
//allocates space for holding strings
char* my_malloc(int _n);
//copies one string to another, allocating space if it isn't present
char* my_strcpy(char *_source, char *_destination);
//initialization function for loginfo_t
void loginfo_init(loginfo_t *_loginfo);
//frees resources used by the logger
void free_resources(char *_cmd, char ** _args, int _argc, char *_buffer, loginfo_t *_loginfo);





#endif
