/*CHANGES :





*/

/*
    Controller source code.
    This program is invoked by the shell for each ;-divided string representing a sequence of commands to be executed.
    For such sting the programm will :
      1)Input tokenization:
        this step transform a generic string in a sequence of tokens, checking for syntax and lessical errors
      2)Programms execution:
        everytime the algorithm sees fit to spawn a logger it will do so, passing information relevant to that programm execution.
      3)Information logging:
        once a logger's job is done it notifies the Controller
        the controller will in tourn gather the loggers results and save theme to the specified file
      4)Clean-up:
        Once every logger child has completed the controller will perform clean-up and return relevant information to the shells

*/
#include "std_libraries.h"
#include "macros.h"
#include "types.h"
#include "functions.h"


//argv[0] always eq "controller"
//argv[1] string containing controller's input
int main(int argc,char **argv){
  printf("argv[1] = %s\n",argv[1]);
  char rawInput[MAX_CMD_LEN];
  strcpy(rawInput,argv[1]);
  int nTokens = 0;
  token_t **inputTokens = tokenize(rawInput,&nTokens);
  /*DEBUG PRINT TOKENS RECIVED FROM TOKENIZE*/
  int i = 0;
  while(inputTokens[i] != NULL){
    printToken(inputTokens[i]);
    i++;
  }

  //Puntatore al token in analisi
  token_t *currentToken;

  //Puntatore all'indice del token corrente
  int currentPointer = 0;

  //Flag di errore
  bool error = FALSE;

  //Lista delle pipe
  pipesList_t *pipesHead,*pipesTail;
  pipesHead = NULL;
  pipesTail = NULL;

  //Lista delle tabelle dei processi
  processesList_t *processesListHead,*processesListTail;
  processesListHead = NULL;
  processesListTail = NULL;

  //Tabelle dei processi
  processTable_t *currentProcessTable,*nextProcessTable;
  currentProcessTable = malloc(sizeof(processTable_t));
  nextProcessTable = malloc(sizeof(processTable_t));
  clearTable(currentProcessTable);
  clearTable(nextProcessTable);

  while( (currentPointer < nTokens) && !error ){
    //Leggo un token
    currentToken = inputTokens[currentPointer];
    //Controllo sul TIPO di token
    switch(currentToken->type){

      case COMMAND :

        printf("token : <%d,%s> riconosciuto come COMANDO\n",currentToken->type,(char*)currentToken->value);
        if(strcmp(currentProcessTable->command,"") != 0)  { exit_w(ERR_CMD_EXSTS);}
        strcpy(currentProcessTable->command,currentToken->value);
        char tmpName[MAX_CMD_LEN];

        strcpy(tmpName,"/tmp/");
        strcat(tmpName,currentToken->value);
        strcat(tmpName,"outFile_XXXXXX");
        currentProcessTable->tmpOutFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpOutFile,tmpName);

        strcpy(tmpName,"/tmp/");
        strcat(tmpName,currentToken->value);
        strcat(tmpName,"errFile_XXXXXX");
        currentProcessTable->tmpErrFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpErrFile,tmpName);

        strcpy(tmpName,"/tmp/");
        strcat(tmpName,currentToken->value);
        strcat(tmpName,"procInfoFile_XXXXXX");
        currentProcessTable->tmpProcInfoFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpProcInfoFile,tmpName);


        currentProcessTable->nOptions = 0;
        break;

      case OPTION :

        printf("token : <%d,%s> riconosciuto come OPZIONE\n",currentToken->type,(char*)currentToken->value);
        if(currentProcessTable->nOptions > MAX_ARGUMENTS) {  exit_w(ERR_MAX_ARGS); }
        if( strcmp(currentProcessTable->options[currentProcessTable->nOptions],"") != 0)  { exit_w(ERR_OPT_EXSTS); }
        strcpy(currentProcessTable->options[currentProcessTable->nOptions],currentToken->value);
        currentProcessTable->nOptions += 1;
        break;

      case OPERATOR :

        printf("token : <%d,%ld> riconosciuto come OPERATORE\n",currentToken->type,(long)currentToken->value);
        switch(*((int*)(currentToken -> value))){

          case IN_REDIRECT:{

            printf("operatore riconosciuto come IN_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken->type != FILE){exit_w(ERR_FILE_XPCTD);}
            strcpy(currentProcessTable->inputFile,fileToken->value);
            currentProcessTable->inputPipe = open_w(currentProcessTable->inputFile);
            currentPointer += 1;

            }
          break;

          case OUT_REDIRECT:{

            printf("operatore riconosciuto come OUT_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken->type != FILE){exit_w(ERR_FILE_XPCTD);}
            strcpy(currentProcessTable->outRedirectFile,fileToken->value);
            currentProcessTable->outRedirectFD = open_w(currentProcessTable->outRedirectFile);
            currentPointer += 1;

          }
          break;

          case PIPE:{
            printf("operatore riconosciuto come PIPE\n");
            pipesList_t *newPipe = malloc(sizeof(pipesList_t));
            pipe_w(newPipe->pipe);
            currentProcessTable->outputPipe = newPipe->pipe[WRITE];
            nextProcessTable->inputPipe = newPipe->pipe[READ];
            pushToPipesList(&pipesHead,&pipesTail,newPipe);
            if(currentProcessTable->skip == FALSE){
              currentProcessTable->pid = fork();
              if(currentProcessTable->pid == 0){

                int argv_size = 4 + currentProcessTable->nOptions;
                char ** exec_argv = malloc(argv_size * sizeof(char*));
                //Copio nome eseguibile
                exec_argv[0] = malloc(sizeof(char)*MAX_ARG_LEN);
                strcpy(exec_argv[0],"logger\0");
                //Copio comando da eseguire
                exec_argv[1] = malloc(sizeof(char)*MAX_ARG_LEN);
                strcpy(exec_argv[1],currentProcessTable->command);
                //Copio numero opzioni
                exec_argv[2] = malloc(sizeof(char)*MAX_ARG_LEN);
                snprintf(exec_argv[2],sizeof(exec_argv[2]),"%d",currentProcessTable->nOptions);
                //Copio opzioni
                int i;
                for(i=3;i<(argv_size-1);i++){
                  exec_argv[i] = malloc(sizeof(char)*MAX_ARG_LEN);
                  strcpy(exec_argv[i],currentProcessTable->options[i-3]);
                }
                //Copio NULL in ultima posizione
                exec_argv[argv_size-1] = NULL;
                setenv_w("outFile",currentProcessTable->tmpOutFile);
                setenv_w("errFile",currentProcessTable->tmpErrFile);
                setenv_w("procInfo",currentProcessTable->tmpProcInfoFile);
                setenv_wi("inputPipe",currentProcessTable->inputPipe);
                setenv_wi("outputPipe",currentProcessTable->outputPipe);
                execv("logger",exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable->pid > 0){}
              else{
                exit_w(ERR_FORK_FAIL);
              }
            }
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable->table = malloc(sizeof(processTable_t));
            copyTable(newTable->table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);

            }
            break;
          case AND:{

            printf("operatore riconosciuto come AND\n");
            if(currentProcessTable->skip == FALSE){
              currentProcessTable->pid = fork();
              if(currentProcessTable->pid == 0){
                int argv_size = 4 + currentProcessTable->nOptions;
                char ** exec_argv = malloc(argv_size * sizeof(char*));
                //Copio nome eseguibile
                exec_argv[0] = malloc(sizeof(char)*MAX_ARG_LEN);
                strcpy(exec_argv[0],"logger\0");
                //Copio comando da eseguire
                exec_argv[1] = malloc(sizeof(char)*MAX_ARG_LEN);
                strcpy(exec_argv[1],currentProcessTable->command);
                //Copio numero opzioni
                exec_argv[2] = malloc(sizeof(char)*MAX_ARG_LEN);
                snprintf(exec_argv[2],sizeof(exec_argv[2]),"%d",currentProcessTable->nOptions);
                //Copio opzioni
                int i;
                for(i=3;i<(argv_size-1);i++){
                  exec_argv[i] = malloc(sizeof(char)*MAX_ARG_LEN);
                  strcpy(exec_argv[i],currentProcessTable->options[i-3]);
                }
                //Copio NULL in ultima posizione
                exec_argv[argv_size-1] = NULL;
                setenv_w("outFile",currentProcessTable->tmpOutFile);
                setenv_w("errFile",currentProcessTable->tmpErrFile);
                setenv_w("procInfo",currentProcessTable->tmpProcInfoFile);
                setenv_wi("inputPipe",currentProcessTable->inputPipe);
                setenv_wi("outputPipe",currentProcessTable->outputPipe);
                execv("logger",exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable->pid > 0){
                int wstatus;
                waitpid_w(currentProcessTable->pid,&wstatus,0);
                if(WIFEXITED(wstatus)){
                  int status = WEXITSTATUS(wstatus);
                  if(status < 0){
                    nextProcessTable->skip = TRUE;
                  }
                }
              }
              else{
                exit_w(ERR_FORK_FAIL);
              }
            }
            else if(currentProcessTable->skip == TRUE){
              nextProcessTable->skip = currentProcessTable->skip && FALSE;
            }
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable->table = malloc(sizeof(processTable_t));
            copyTable(newTable->table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);

            }
            break;
          case OR:{

            printf("operatore riconosciuto come OR\n");
            if(currentProcessTable->skip == FALSE){
              currentProcessTable->pid = fork();
              if(currentProcessTable->pid == 0){
                int argv_size = 4 + currentProcessTable->nOptions;
                char ** exec_argv = malloc(argv_size * sizeof(char*));
                //Copio nome eseguibile
                exec_argv[0] = malloc(sizeof(char)*MAX_ARG_LEN);
                strcpy(exec_argv[0],"logger\0");
                //Copio comando da eseguire
                exec_argv[1] = malloc(sizeof(char)*MAX_ARG_LEN);
                strcpy(exec_argv[1],currentProcessTable->command);
                //Copio numero opzioni
                exec_argv[2] = malloc(sizeof(char)*MAX_ARG_LEN);
                snprintf(exec_argv[2],sizeof(exec_argv[2]),"%d",currentProcessTable->nOptions);
                //Copio opzioni
                int i;
                for(i=3;i<(argv_size-1);i++){
                  exec_argv[i] = malloc(sizeof(char)*MAX_ARG_LEN);
                  strcpy(exec_argv[i],currentProcessTable->options[i-3]);
                }
                //Copio NULL in ultima posizione
                exec_argv[argv_size-1] = NULL;
                setenv_w("outFile",currentProcessTable->tmpOutFile);
                setenv_w("errFile",currentProcessTable->tmpErrFile);
                setenv_w("procInfo",currentProcessTable->tmpProcInfoFile);
                setenv_wi("inputPipe",currentProcessTable->inputPipe);
                setenv_wi("outputPipe",currentProcessTable->outputPipe);
                execv("logger",exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable->pid > 0){
                int wstatus;
                waitpid_w(currentProcessTable->pid,&wstatus,0);
                if(WIFEXITED(wstatus)){
                  int status = WEXITSTATUS(wstatus);
                  if(status > 0){
                    nextProcessTable->skip = TRUE;
                  }
                }
              }
              else{
                exit_w(ERR_FORK_FAIL);
              }
            }
            else if(currentProcessTable->skip == TRUE){
              nextProcessTable->skip = currentProcessTable->skip || FALSE;
            }
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable->table = malloc(sizeof(processTable_t));
            copyTable(newTable->table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);

            }
            break;

          break;

        }
        break;

      case FILE :
        printf("token : <%d,%s> riconosciuto come FILE\n",currentToken->type,(char*)currentToken->value);
        break;

    }
    currentPointer += 1;
    if(currentPointer == nTokens){
      if(currentProcessTable->skip == FALSE){
        currentProcessTable->pid = fork();
        if(currentProcessTable->pid == 0){

          int argv_size = 4 + currentProcessTable->nOptions;
          char ** exec_argv = malloc(argv_size * sizeof(char*));
          //Copio nome eseguibile
          exec_argv[0] = malloc(sizeof(char)*MAX_ARG_LEN);
          strcpy(exec_argv[0],"logger\0");
          //Copio comando da eseguire
          exec_argv[1] = malloc(sizeof(char)*MAX_ARG_LEN);
          strcpy(exec_argv[1],currentProcessTable->command);
          //Copio numero opzioni
          exec_argv[2] = malloc(sizeof(char)*MAX_ARG_LEN);
          snprintf(exec_argv[2],sizeof(exec_argv[2]),"%d",currentProcessTable->nOptions);
          //Copio opzioni
          int i;
          for(i=3;i<(argv_size-1);i++){
            exec_argv[i] = malloc(sizeof(char)*MAX_ARG_LEN);
            strcpy(exec_argv[i],currentProcessTable->options[i-3]);
          }
          //Copio NULL in ultima posizione
          exec_argv[argv_size-1] = NULL;
          setenv_w("outFile",currentProcessTable->tmpOutFile);
          setenv_w("errFile",currentProcessTable->tmpErrFile);
          setenv_w("procInfo",currentProcessTable->tmpProcInfoFile);
          setenv_wi("inputPipe",currentProcessTable->inputPipe);
          setenv_wi("outputPipe",currentProcessTable->outputPipe);
          execv("logger",exec_argv);
          exit_w(ERR_EXEC_FAIL);
        }
        else if(currentProcessTable->pid > 0){}
        else{
          exit_w(ERR_FORK_FAIL);
        }
      }
      processesList_t *newTable = malloc(sizeof(processesList_t));
      newTable->table = malloc(sizeof(processTable_t));
      copyTable(newTable->table,currentProcessTable);
      pushToTablesList(&processesListHead,&processesListTail,newTable);
      copyTable(currentProcessTable,nextProcessTable);
      clearTable(nextProcessTable);
    }
  }

  printTablesList(processesListHead,processesListTail);
  printPipesList(pipesHead,pipesTail);
}
