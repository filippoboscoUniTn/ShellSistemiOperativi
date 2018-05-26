#include "std_libraries.h"
#include "functions.h"
#include "macros.h"
#include "types.h"

//function for keep the main logic clean and because it can be called multiple times
void printUsage( char* _prog_name ){
	printf("usage: %s [-o | --outfile <file>] [-e | --errfile <file>] [-m | --maxlen <value>]\n	", _prog_name);
	exit(EXIT_SUCCESS);
}

//function to dinamically return arguments codes
//matches the string with every argument
//returns error if the string doesn't match any argument
argcode_t getcode(char *_arg){
	if(strcmp(_arg, ARG_STDOUTFILE_SS) == 0){ return ARG_STDOUTFILE_C; }
	if(strcmp(_arg, ARG_STDOUTFILE_SL) == 0){ return ARG_STDOUTFILE_C; }
	if(strcmp(_arg, ARG_STDERRFILE_SS) == 0){ return ARG_STDERRFILE_C; }
	if(strcmp(_arg, ARG_STDERRFILE_SL) == 0){ return ARG_STDERRFILE_C; }
	if(strcmp(_arg, ARG_MAXLEN_SS)     == 0){ return ARG_MAXLEN_C; }
	if(strcmp(_arg, ARG_MAXLEN_SL)     == 0){ return ARG_MAXLEN_C; }

	return ARG_CODE_ERROR;
}

argtype_t gettype(argcode_t _argcode){

	//type string
	if(_argcode > 0 && _argcode < ARG_MAXLEN_C){
		return ARG_TYPE_S;
	}

	//type int
	if(_argcode >= ARG_MAXLEN_C && _argcode < ARG_SWITCH_C){
		return ARG_TYPE_I;
	}

	//type null
	else{
		return ARG_TYPE_N;
	}
}


//Error handling functions
char * getErrorMessage(int errno){
  char errMessage [MAX_CMD_LEN];
  switch (errno) {
    case ERR_PIPE:
    strcpy(errMessage,"Error opening pipe\0");
    break;

    case ERR_MKSTEMP:
    strcpy(errMessage,"Error creating temporary file\0");
    break;

    case ERR_OPEN:
    strcpy(errMessage,"Error opening file\0");
    break;

    case ERR_WAIT:
    strcpy(errMessage,"Error waiting for child\0");
    break;

    case ERR_SETENV:
    strcpy(errMessage,"Error setting environmental variable for string value\0");
    break;

    case ERR_SETENV_I:
    strcpy(errMessage,"Error setting environmental variable for integer value\0");
    break;

    default:
    strcpy(errMessage,"Error recognizing error : errception\0");
  }
}

void printError(int errno){
  switch (errno) {
    case ERR_PIPE:
    printf("Error opening pipe\n");
    break;

    case ERR_MKSTEMP:
    printf("Error creating temporary file\n");
    break;

    case ERR_OPEN:
    printf("Error opening file\n");
    break;

    case ERR_WAIT:
    printf("Error waiting for child\n");
    break;

    case ERR_SETENV:
    printf("Error setting environmental variable for string value\n");
    break;

    case ERR_SETENV_I:
    printf("Error setting environmental variable for integer value\n");
    break;

    default:
    printf("Error recognizing error : errception\n");
  }
}


//Wrapper functions for system calls
void pipe_w(int *pipes){
  int retVal;
  retVal = pipe(pipes);
  if(retVal < 0){
		exit(ERR_PIPE);
  }
}

int mkstemp_w(char * template){
  int tmpFD = mkstemp(template);
  if(tmpFD < 0){
    exit(ERR_MKSTEMP);
  }
  return tmpFD;
}

void setenv_w(char * key,char * value){
  int setenv_status;
  setenv_status = setenv(key,value,5);
  if(setenv_status < 0){
    exit(ERR_SETENV);
  }
}

void setenv_wi(char * key,int value){
  int setenv_status;
  char intAlpha [MAX_CMD_LEN];
  snprintf(intAlpha,sizeof(intAlpha),"%d",value);
  setenv_status = setenv(key,intAlpha,5);
  if(setenv_status < 0){
    exit(ERR_SETENV_I);
  }
}

void waitpid_w(int pid,int *wstatus,int flag){
  int wait_status = waitpid(pid,wstatus,flag);
  if(wait_status < 0){
    exit(ERR_WAIT);
  }
}

int open_w(char *path){
  int fd;
  fd = open(path,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
  if(fd < 0){
    exit(ERR_OPEN);
  }
  return fd;
}


//Controller's utility functions
void clearTable(processTable_t *table){
  strcpy(table->command,"");
	table->nOptions = -1;
	int i;
	for(i=0;i<table->nOptions;i++){
		strcpy(table->options[i],"");
	}

	strcpy(table->tmpOutFile,"");
	table->tmpOutFD = -1;

	strcpy(table->tmpErrFile,"");
	table->tmpErrFD = -1;

	strcpy(table->tmpProcInfoFile,"");
	table->tmpProcInfoFD = -1;

	strcpy(table->outRedirectFile,"");
	table->outRedirectFD = -1;

	strcpy(table->inputFile,"");
	table->inputPipe = -1;
	table->outputPipe = -1;

	table->pid = -1;
	table->skip = FALSE;

}

void copyTable(processTable_t *tableTo,processTable_t *tableFrom){

	strcpy(tableTo->command,tableFrom->command);
	tableTo->nOptions = tableFrom->nOptions;
	int i;
	for(i=0;i<tableFrom->nOptions;i++){
		strcpy(tableTo->options[i],"");
		strcpy(tableTo->options[i],tableFrom->options[i]);
	}
	strcpy(tableTo->tmpOutFile,tableFrom->tmpOutFile);
	tableTo->tmpOutFD = tableFrom->tmpOutFD;

	strcpy(tableTo->tmpErrFile,tableFrom->tmpErrFile);
	tableTo->tmpErrFD = tableFrom->tmpErrFD;

	strcpy(tableTo->tmpProcInfoFile,tableFrom->tmpProcInfoFile);
	tableTo->tmpProcInfoFD = tableFrom->tmpProcInfoFD;

	strcpy(tableTo->outRedirectFile,tableFrom->outRedirectFile);
	tableTo->outRedirectFD = tableFrom->outRedirectFD;

	strcpy(tableTo->inputFile,tableFrom->inputFile);
	tableTo->inputPipe = tableFrom->inputPipe;
	tableTo->outputPipe = tableFrom->outputPipe;

	tableTo->pid = tableFrom->pid;
	tableTo->skip = tableFrom->skip;

	return;
}

void pushToPipesList(pipesList_t **head,pipesList_t **tail,pipesList_t *newElement){
  if(*tail == NULL && *head == NULL){
    newElement->prev = NULL;
    newElement->next = NULL;
    *tail = newElement;
    *head = newElement;
  }
  else{
    (*head)->next = newElement;
    newElement->prev = *head;
    newElement->next = NULL;
    *head = newElement;
  }
  return;
}

void pushToTablesList(processesList_t **head,processesList_t **tail,processesList_t *newElement){
  if(*tail == NULL && *head == NULL){
    newElement->prev = NULL;
    newElement->next = NULL;
    *tail = newElement;
    *head = newElement;
  }
  else{
    (*head)->next = newElement;
    newElement->prev = *head;
    newElement->next = NULL;
    *head = newElement;
  }
  return;
}


//Print functions
void printTablesList(processesList_t *head,processesList_t *tail){
printf("\nPRINTING TABLES LIST\n");
  int counter = 0;
  while(tail != NULL){

    printf("\tTable[%d] :\n\t\ttail->command = %s\n\t\t",counter,tail->table->command);
		printf("Options :\n");
		int i;
		for(i=0;i<tail->table->nOptions;i++){
			printf("\t\t\toption[%d] = %s\n",i,tail->table->options[i]);
		}
		printf("\t\ttmpOutFile = %s\n\t\ttmpOutFD = %d\n",tail->table->tmpOutFile,tail->table->tmpOutFD);
		printf("\t\ttmpErrFile = %s\n\t\ttmpErrFD = %d\n",tail->table->tmpErrFile,tail->table->tmpErrFD);
		printf("\t\ttmpProcInfoFile = %s\n\t\ttmpProcInfoFD = %d\n",tail->table->tmpProcInfoFile,tail->table->tmpProcInfoFD);
		printf("\t\toutRedirectFile = %s\n\t\toutRedirectFD = %d\n",tail->table->outRedirectFile,tail->table->outRedirectFD);
		printf("\t\tinputPipe = %d\n\t\tinputFile = %s\n\t\toutputPipe = %d\n",tail->table->inputPipe,tail->table->inputFile,tail->table->outputPipe);
		printf("\t\tpid = %d\n\t\tskip = %d\n",tail->table->pid,tail->table->skip);

    tail = tail->next;
    counter ++;
  }
	printf("\n");

	return;
}

void printPidsList(pidsList_t *head,pidsList_t *tail){
printf("\nPRINTING PIDS LIST\n");
  int counter = 0;
  while(tail != NULL){
    printf("\tPidsList[%d] :\n\t\tpid = %d\n",counter,tail->pid);
    tail = tail->next;
		counter ++;
  }
	printf("\n");

	return;
}

void printPipesList(pipesList_t *head,pipesList_t *tail){
printf("\nPRINTING PIPES LIST\n");
  int counter = 0;
  while(tail != NULL){
    printf("\tPipeList[%d] :\n\t\tpipe[READ] = %d\n\t\tpipe[WRITE] = %d\n",counter,tail->pipe[READ],tail->pipe[WRITE]);
    tail = tail->next;
		counter ++;
  }
printf("\n");
return;
}
