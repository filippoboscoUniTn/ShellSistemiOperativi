//include guard
#ifndef TYPES__H_
#define TYPES__H_

#include "macros.h"

//boolean type definition
typedef int bool;

//argcode type definition
typedef int argcode_t;

//argument type definition
typedef int argtype_t;

//argument type
typedef struct{
	argcode_t arg_code; //the argument's specific code for identification
	void *arg_value; //based on the argcode, this will be allocated for an int, char* or whatever
} arg_t;

//tokentType definition
typedef int tokenType_t;

//Token type
typedef struct token_t{
	tokenType_t type;
	void* value;
}token_t;

typedef struct processTable_t{

	char command [MAX_CMD_LEN];

	int nOptions;
	char options [MAX_ARGUMENTS] [MAX_ARG_LEN];

	char tmpOutFile [MAX_FILE_NAME_LEN];
	int tmpOutFD;

	char tmpErrFile [MAX_FILE_NAME_LEN];
	int tmpErrFD;

	char tmpProcInfoFile [MAX_FILE_NAME_LEN];
	int tmpProcInfoFD;

	char outRedirectFile [MAX_FILE_NAME_LEN];
	int outRedirectFD;

	char inputFile [MAX_FILE_NAME_LEN];
	int inputPipe;
	int outputPipe;

	int pid;
	bool skip;

}processTable_t;

typedef struct processesList_t{
	struct processTable_t *table;

	struct processesList_t *next;
	struct processesList_t *prev;

}processesList_t;

// typedef struct token_told{
//   char type[10];
//   char value[50];
// }token_told;

// //Process Table type definition
// typedef struct processIOtable_t{
//   char command [CMD_EXP_BUFF_SIZE];
//   char options [MAX_ARGUMENTS][MAX_ARG_LEN];
// 	int nOptions;
// 	char logFilePath [LOG_FILE_NAME_LEN];
//   int inputPipe;
//   int outputPipe;
//   char inputFile[MAX_IN_FILE_LEN];
//   char outputFile[MAX_OUT_FILE_LEN];
// 	bool skipNextCommand;
// }processIOtable_t;

//Pids List type definition
typedef struct pidsList_t{
  pid_t pid;
  struct pidsList_t *next;
  struct pidsList_t *prev;
}pidsList_t;

//Pipe List type definition
typedef struct pipesList_t{
  int pipe[2];
  struct pipesList_t *next;
  struct pipesList_t *prev;
}pipesList_t;


// //Processes Tables' List definition
// typedef struct processesList_t{
//   struct processIOtable_t *processTable;
//   struct processesList_t *next;
//   struct processesList_t *prev;
// }processesList_t;


#endif
