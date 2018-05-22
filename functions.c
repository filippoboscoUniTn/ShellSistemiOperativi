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


//Controller's help functions
void swapTables(processIOtable_t *currentProcessTable,processIOtable_t *nextProcessTable){
  //Copia della tabella nextProcessTable in currentProcessTable
  strcpy(currentProcessTable->command,nextProcessTable->command);
  int i;
  for(i=0;i<MAX_ARGUMENTS;i++){
    //Ci metto una stringa vuota cosi che se nextProcessTable.options < currentProcessTable.options quello che sarà poi il currentProcessTable non si ritrova per sbaglio argomenti di qualche comando passato
    strcpy(currentProcessTable->options[i],"");
    strcpy(currentProcessTable->options[i],nextProcessTable->options[i]);
  }
  currentProcessTable->inputPipe = nextProcessTable->inputPipe;
  currentProcessTable->outputPipe = -1;
  strcpy(currentProcessTable->inputFile,"");
  strcpy(currentProcessTable->outputFile,"");
}


// void swapAndPushTables(processIOtable_t *currentProcessTable, processIOtable_t *nextProcessTable, processIOtable_t *processesTables,int* processCounter){
//   processIOtable_t *processTableTmp;
//   processTableTmp = malloc(sizeof(processIOtable_t));
//   //Copio currentProcessTable in processTableTmp
//   strcpy(processTableTmp->command,currentProcessTable->command);
//   strcpy(processTableTmp->inputFile,currentProcessTable->inputFile);
//   strcpy(processTableTmp->outputFile,currentProcessTable->outputFile);
//   processTableTmp->outputPipe = currentProcessTable->outputPipe;
//   processTableTmp->inputPipe = currentProcessTable->inputPipe;
//   processesTables[*processCounter] = *processTableTmp;
//   processCounter += 1;
//   //Copia della tabella nextProcessTable in currentProcessTable
//   strcpy(currentProcessTable->command,nextProcessTable->command);
//   int i;
//   for(i=0;i<MAX_ARGUMENTS;i++){
//     //Ci metto una stringa vuota cosi che se nextProcessTable.options < currentProcessTable.options quello che sarà poi il currentProcessTable non si ritrova per sbaglio argomenti di qualche comando passato
//     strcpy(currentProcessTable->options[i],"");
//     strcpy(currentProcessTable->options[i],nextProcessTable->options[i]);
//   }
//   currentProcessTable->inputPipe = nextProcessTable->inputPipe;
//   currentProcessTable->outputPipe = -1;
//   strcpy(currentProcessTable->inputFile,"");
//   strcpy(currentProcessTable->outputFile,"");
// }


void clearTable(processIOtable_t *table){
  strcpy(table->command,"");
  strcpy(table->inputFile,"");
  strcpy(table->outputFile,"");
  table->outputPipe = -1;
  table->inputPipe = -1;
	table->skipNextCommand = FALSE;
}


void pushToPidsList(pidsList_t **head,pidsList_t **tail,pidsList_t *newElement){
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


void printTablesList(processesList_t *head,processesList_t *tail){
  int counter = 0;
  while(tail != NULL){
    printf("Table[%d] :\n\ttail->command = %s\n\ttail->inputPipe = %d\n\ttail->outputPipe = %d",counter,tail->processTable->command,tail->processTable->inputPipe,tail->processTable->outputPipe);
    printf("\ttail->outputFile = %s\n\ttail->inputFile = %s",tail->processTable->inputFile,tail->processTable->outputFile);
    tail = tail->next;
    counter ++;
  }
}


void printPidsList(pidsList_t *head,pidsList_t *tail){
  int counter = 0;
  while(tail != NULL){
    printf("PidsList[%d] :\n\tpid = %d",counter,tail->pid);
    tail = tail->next;
  }
}


void printPipesList(pipesList_t *head,pipesList_t *tail){
  int counter = 0;
  while(tail != NULL){
    printf("PipeList[%d] :\n\tpipe[READ] = %d\n\tpipe[WRITE] = %d\n",counter,tail->pipe[READ],tail->pipe[WRITE]);
    tail = tail->next;
  }
}
