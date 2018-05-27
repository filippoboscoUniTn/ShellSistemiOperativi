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

bool isOperator(char* inputString,token_t*tokenTmp){
  printf("strlen(inputString) == %ld\n",strlen(inputString));
  printf("in isOperator, inputString = %s\n",inputString);
  printf("strcmp(inputString,'|') == 0 -> %d\n", strcmp(inputString,"|") == 0);
  printf("inputString[0] == '|' -> %d\n", (inputString[0] == '|') );
  printf("inputString[1] == zero -> %d\n",inputString[0] == '\0');
  bool retVal = FALSE;
    if(inputString[0] == '|' && inputString[1] == '\0'){
      printf("matched as PIPE\n");
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      *(int*)(tokenTmp->value) = PIPE;
      tokenTmp -> value = PIPE;
    }
    else if(strcmp(inputString,"|") == 0){
      printf("matched PIPE\n");
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      *(int*)(tokenTmp->value) = PIPE;
      tokenTmp -> value = PIPE;

    }
    else if(strcmp(inputString,"&&") == 0){
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      *(int*)(tokenTmp->value) = AND;
    }
    else if(strcmp(inputString,"||") == 0){
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      *(int*)(tokenTmp->value) = OR;
    }
    else if(strcmp(inputString,"<") == 0){
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      *(int*)(tokenTmp->value) = IN_REDIRECT;
    }
    else if(strcmp(inputString,">") == 0){
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      *(int*)(tokenTmp->value) = OUT_REDIRECT;
    }


  return retVal;
}

token_t ** tokenize(char *rawInput){

  token_t * tokenArray[MAX_CMD_LEN];
  char words[MAX_CMD_LEN][MAX_CMD_LEN];
  int wordCounter = 0;
  int charPointer = 0;
  int nTokens = 0;
  int subStringPointer = 0;
  char subString[MAX_ARG_LEN];
  char currentCharacter[2];

  currentCharacter[0] = rawInput[charPointer];
  currentCharacter[1] = '\0';
  //Flag che specifica se l'ultimo token incontrato era :
  bool command = FALSE; //Un comando
  bool redirect = FALSE; //Un operatore di redirezione (i.e. <,>)
  bool operator = FALSE; //Un operatore (i.e. |,||,&&)

  //Itera fino a fine input
  while( (charPointer < MAX_CMD_LEN) && (strcmp(currentCharacter,"\0")!= 0) ){
    //Itera fino ad uno spazio o fine input o EOF
    while( (charPointer<MAX_CMD_LEN) && (strcmp(currentCharacter," ")!= 0) && (strcmp(currentCharacter,"\0")!= 0) ){
      subString[subStringPointer] = currentCharacter[0];
      subStringPointer++;
      charPointer++;
      printf("\tcurrentCharacter[0] = %c\n",  currentCharacter[0] );
      currentCharacter[0] = rawInput[charPointer];
    }
    //Salta lo spazio
    charPointer ++;
    currentCharacter[0] = rawInput[charPointer];
    printf("subString recognized : %s\n",subString);
    printf("strlen(subString) = %ld\n",strlen(subString));
    strcpy(words[wordCounter],subString);
    strcat(words[wordCounter],"\0");
    printf("strcmp words[wordCounter+1] and zero -> %d\n",strcmp(words[wordCounter],"\0"));
    //printf("strcmp words[wordCounter+1] and zero -> %d\n",strcmp(words[wordCounter],'\0'));


    strcpy(subString,"");
    subStringPointer = 0;

    //Aggiornamento variabili di ciclo
    printf("words[wordCounter] = %s\n",words[wordCounter]);
    //Incontrato uno spazio -> riconosciuta una parola
    token_t * tokenTmp = malloc(sizeof(token_t));
    printf("goin to match : words[%d] = %s\n",wordCounter,words[wordCounter]);
    bool isOp = isOperator(words[wordCounter],tokenTmp);
    //Controllo se è un operatore
    if(isOp){
      if(operator){
        printf("Error : 2 consecutive operators found\n");
        exit(EXIT_FAILURE);
      }
      else if(redirect){
        printf("Errore : necessario il nome di un file dopo un operatore di redirezione\n");
        exit(EXIT_FAILURE);
      }
      else{
        tokenArray[nTokens] = tokenTmp;
        nTokens++;
        operator = TRUE;
        command = FALSE;
        redirect = FALSE;
      }
    }
    //NOn è un operatore
    else{
      //L'ultimo token era un operatore di redirezione -> sto leggendo un token FILE
      if(redirect){
        //Match as fileName
        printf("matching token as file name\n\n");
        tokenTmp -> type = FILE;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordCounter]);
        tokenArray[nTokens] = tokenTmp;
        nTokens++;
        redirect = TRUE;
        command = FALSE;
        operator = FALSE;
      }
      //L'ultimo token era un comando -> sto leggendo un opzione
      else if(command){
        printf("matching token as option\n\n");
        //Match as option
        tokenTmp -> type = OPTION;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordCounter]);
        tokenArray[nTokens] = tokenTmp;
        nTokens++;
        command = TRUE;
        operator = FALSE;
        redirect = FALSE;
      }
      else{
        //Match as command
        printf("matching token as command\n\n");
        tokenTmp -> type = COMMAND;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordCounter]);
        tokenArray[nTokens] = tokenTmp;
        nTokens++;
        command = TRUE;
        operator = FALSE;
        redirect = FALSE;
      }
    }
    wordCounter ++;
  }


  //Stampa parole riconosciute
  int i;
  for(i=0;i<wordCounter;i++){
    printf("words[%d] = %s\n",i,words[i]);
  }

  exit(EXIT_FAILURE);
  //Finito di leggere un token provo a matcharlo
  token_t * tokenTmp = malloc(sizeof(token_t));
  if(isOperator(subString,tokenTmp)){
    printf("token recognized\n");
  }
  return tokenArray;
}




//argv[0] always eq "controller"
//argv[1] string containing controller's input
int main(int argc,char **argv){
  char rawInput[MAX_CMD_LEN];
  strcpy(rawInput,argv[1]);
  token_t **inputTokensTmp = tokenize(rawInput);
  exit(EXIT_SUCCESS);
  //Example INPUT
  token_t token_0;
    token_0.type = COMMAND;
    token_0.value = malloc(MAX_CMD_LEN*sizeof(char));
    strcpy(token_0.value,"ls");
  token_t token_1;
    token_1.type = OPTION;
    token_1.value = malloc(MAX_CMD_LEN*sizeof(char));
    strcpy(token_1.value,"-l");
  token_t token_2;
    token_2.type = OPTION;
    token_2.value = malloc(MAX_CMD_LEN*sizeof(char));
    strcpy(token_2.value,"-o");
  token_t token_3;
    token_3.type = OPERATOR;
    token_3.value = malloc(sizeof(int));
    token_3.value = PIPE;
  token_t token_4;
    token_4.type = COMMAND;
    token_4.value = malloc(MAX_CMD_LEN*sizeof(char));
    strcpy(token_4.value,"wc");


  int nTokens = 5;
  token_t inputTokens [] = {token_0,token_1,token_2,token_3,token_4};

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
    currentToken = &inputTokens[currentPointer];
    //Controllo sul TIPO di token
    switch(currentToken->type){

      case COMMAND :

        printf("token : <%d,%s> riconosciuto come COMANDO\n",currentToken->type,(char*)currentToken->value);
        if(strcmp(currentProcessTable->command,"") != 0)  { exit(EXIT_FAILURE);}
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
        if(currentProcessTable->nOptions > MAX_ARGUMENTS) {  exit(EXIT_FAILURE); }
        if( strcmp(currentProcessTable->options[currentProcessTable->nOptions],"") != 0)  { exit(EXIT_FAILURE); }
        strcpy(currentProcessTable->options[currentProcessTable->nOptions],currentToken->value);
        currentProcessTable->nOptions += 1;
        break;

      case OPERATOR :

        printf("token : <%d,%ld> riconosciuto come OPERATORE\n",currentToken->type,(long)currentToken->value);
        switch((long)currentToken->value){

          case IN_REDIRECT:{

            printf("operatore riconosciuto come IN_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = &inputTokens[nextPointer];
            if(fileToken->type != FILE){exit(EXIT_FAILURE);}
            strcpy(currentProcessTable->inputFile,fileToken->value);
            currentProcessTable->inputPipe = open_w(currentProcessTable->inputFile);
            currentPointer += 1;

            }
          break;

          case OUT_REDIRECT:{

            printf("operatore riconosciuto come OUT_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = &inputTokens[nextPointer];
            if(fileToken->type != FILE){exit(EXIT_FAILURE);}
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
                exit(EXIT_FAILURE);
              }
              else if(currentProcessTable->pid > 0){}
              else{
                exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
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
          exit(EXIT_FAILURE);
        }
        else if(currentProcessTable->pid > 0){}
        else{
          exit(EXIT_FAILURE);
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
