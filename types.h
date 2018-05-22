//include guard
#ifndef TYPES__H_
#define TYPES__H_


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

//Structs per controller.c

//Token type definition
typedef struct{
  char type[10];
  char value[50];
}token_t;

//Process Table type definition
typedef struct{
  char command [CMD_EXP_BUFF_SIZE];
  char options [MAX_ARGUMENTS][MAX_ARG_LEN];
  int inputPipe;
  int outputPipe;
  char inputFile[MAX_IN_FILE_LEN];
  char outputFile[MAX_OUT_FILE_LEN];
	bool skipNextCommand;
}processIOtable_t;

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

//Processes Tables' List definition
typedef struct processesList_t{
  struct processIOtable_t *processTable;
  struct processesList_t *next;
  struct processesList_t *prev;
}processesList_t;


#endif
