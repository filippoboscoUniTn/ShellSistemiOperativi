//include guard
#ifndef FUNCTIONS__H_
#define FUNCTIONS__H_





#include "types.h"

void printUsage( char* _prog_name );
argcode_t getcode(char *_arg);
argtype_t gettype(argcode_t _argcode);


void printError(int);
char * getErrorMessage(int);


void pipe_w(int *);
void setenv_w(char * ,char *);
void setenv_wi(char * ,int);
void waitpid_w(int ,int *,int);
int mkstemp_w(char *);
int open_w(char *);


void copyTable(processTable_t *,processTable_t *);
void clearTable(processTable_t *);
void pushToPipesList(pipesList_t **, pipesList_t **, pipesList_t *);
void pushToTablesList(processesList_t **, processesList_t **, processesList_t *);


void printTablesList(processesList_t *, processesList_t *);
void printPidsList(pidsList_t *, pidsList_t *);
void printPipesList(pipesList_t *, pipesList_t *);


#endif
