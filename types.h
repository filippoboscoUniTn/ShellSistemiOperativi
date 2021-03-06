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

typedef struct{

	//these variables are nearly always used
	//based on the arguments switch, we know if allocate memory or not
	char *out_pathname; //stdout partial log file pathname
	char *err_pathname; //stderr partial log file pathname
	char *proc_info_pathname; //proc_info partial log file pathname
	int pipe_in; //input pipe read end which has to be linked to stdin of the executed command
	int pipe_out; //output pipe write end in which the logger redirect stdout of the executed program

	int outf; //stdout partial log file's FD
	int errf; //stderr partial log file's FD
	int proc_infof; //proc_info partial log file's FD

}loginfo_t;


//tokentType definition
typedef int tokenType_t;

//Token type
typedef struct token_t{
	tokenType_t type;
	void* value;
}token_t;

typedef struct processTable_t{
	//Stringa che rappresenta il comando da eseguire
	char command [MAX_CMD_LEN];
	//Numero di opzioni e array di stringhe rappresentanti le opzioni del comando
	int nOptions;
	char options [MAX_ARGUMENTS] [MAX_ARG_LEN];

	//File temporanei per i log parziali
	char tmpOutFile [MAX_FILE_NAME_LEN]; //File di OUTPUT
	char tmpErrFile [MAX_FILE_NAME_LEN]; //File di ERROR
	char tmpProcInfoFile [MAX_FILE_NAME_LEN]; //File contenete le informazioni del processo relativo al comando

	//Nome del File di redirezione dell'output.
	//NB Allo stato corrente del progetto la shell supporta al massimo un operatore di redirezione di output "statico" (i.e. viene popolato a terminazione dei processi relativi alla stringa dei comandi passata al controller)
	char outRedirectFile [MAX_FILE_NAME_LEN];

	//Nome del File di redirezione dell'input
	char inputFile [MAX_FILE_NAME_LEN];

	int inputPipe;
	int outputPipe;

	int pid;
	int status;
	bool skip;

}processTable_t;

typedef struct processesList_t{
	struct processTable_t *table;
	struct processesList_t *next;
	struct processesList_t *prev;

}processesList_t;

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

#endif
