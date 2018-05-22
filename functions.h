//include guard
#ifndef FUNCTIONS__H_
#define FUNCTIONS__H_





#include "types.h"

void printUsage( char* _prog_name );
argcode_t getcode(char *_arg);
argtype_t gettype(argcode_t _argcode);


void swapTables(processIOtable_t *, processIOtable_t *);
//void swapAndPushTables(processIOtable_t *, processIOtable_t *, processIOtable_t *, int*);
void clearTable(processIOtable_t *);

void pushToPidsList(pidsList_t **, pidsList_t **, pidsList_t *);
void pushToPipesList(pipesList_t **, pipesList_t **, pipesList_t *);
void pushToTablesList(processesList_t **, processesList_t **, processesList_t *);

void printTablesList(processesList_t *, processesList_t *);
void printPidsList(pidsList_t *, pidsList_t *);
void printPipesList(pipesList_t *, pipesList_t *);


#endif
