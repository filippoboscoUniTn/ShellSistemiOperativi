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
        once all loggers are done the controller will in tourn gather the loggers results and save theme to the specified file
      4)Clean-up:
        Once every logger child has completed the controller will perform clean-up and return relevant information to the shell

*/

//------------------------- EXTERNAL LIBRARIES INCLUSION -----------------------------
#include "std_libraries.h"
#include "macros.h"
#include "types.h"
#include "functions.h"
//------------------------- EXTERNAL LIBRARIES INCLUSION -----------------------------
//-------------------------------------- END -----------------------------------------


//------------------------- CONTROLLER INVOCATION PARAMETERS -------------------------
//argv[0] -> EXECUTABLE NAME
//argv[1] -> RAW INPUT
//argv[>1] -> UNDEFINED
//------------------------- CONTROLLER INVOCATION PARAMETERS -------------------------
//-------------------------------------- END -----------------------------------------
int main(int argc,char **argv){


//------------------ ARGUMENTS CHEKING FOR CORRECT PROGRAM'S INVOCATION --------------
  if(argc != 2){
    exit_w(ERR_INVCATION_CNT);
  }
//------------------ ARGUMENTS CHEKING FOR CORRECT PROGRAM'S INVOCATION --------------
//-------------------------------------- END -----------------------------------------



//------------------------------ RAW INPUT TOKENIZATION ------------------------------
  int nTokens = 0;
  char rawInput[MAX_CMD_LEN];
  strcpy(rawInput,argv[1]);
  token_t **inputTokens = tokenize(rawInput,&nTokens);
//------------------------------ RAW INPUT TOKENIZATION ------------------------------
//-------------------------------------- END -----------------------------------------



//--------------------- DEBUG PRINT TOKENS RECIVED FROM TOKENIZE ---------------------
  int i = 0;
  while(inputTokens[i] != NULL){
    printToken(inputTokens[i]);
    i++;
  }
//--------------------- DEBUG PRINT TOKENS RECIVED FROM TOKENIZE ---------------------
//-------------------------------------- END -----------------------------------------



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
            if(fileToken->type != FILE_){exit_w(ERR_FILE_XPCTD);}
            strcpy(currentProcessTable->inputFile,fileToken->value);
            currentProcessTable->inputPipe = open_w(currentProcessTable->inputFile);
            currentPointer += 1;

            }
          break;

          case OUT_REDIRECT:{

            printf("operatore riconosciuto come OUT_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken->type != FILE_){exit_w(ERR_FILE_XPCTD);}
            strcpy(currentProcessTable->outRedirectFile,fileToken->value);
            currentProcessTable->outRedirectFD = open_w(currentProcessTable->outRedirectFile);
            currentPointer += 1;

          }
          break;

          case PIPE:{
            printf("operatore riconosciuto come PIPE\n");

//------------------------------ALLOCAZIONE DELLA PIPE -----------------------
//--------PER L'IPC DEGLI OPERANDI A DX E SX DELL OPERATORE DI PIPEE ---------
            pipesList_t *newPipe = malloc(sizeof(pipesList_t));
            pipe_w(newPipe->pipe);
            currentProcessTable->outputPipe = newPipe->pipe[WRITE];
            nextProcessTable->inputPipe = newPipe->pipe[READ];
            pushToPipesList(&pipesHead,&pipesTail,newPipe);
//---------------------------- END -------------------------------------------



            //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
            // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
            //              e l'operando a SX di quest'ultimo ha fallito
            if(currentProcessTable->skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable->pid = fork();
              if(currentProcessTable->pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                setenv_w(EV_STDOUTFILE,currentProcessTable->tmpOutFile);
                setenv_w(EV_STDERRFILE,currentProcessTable->tmpErrFile);
                setenv_w(EV_PINFO_OUTFILE,currentProcessTable->tmpProcInfoFile);
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_NAME,exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable->pid > 0){}
              else{
                exit_w(ERR_FORK_FAIL);
              }
              //---------------------------- END -------------------------------


            }
            //---------------------------- END ---------------------------------



            //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable->table = malloc(sizeof(processTable_t));
            copyTable(newTable->table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);
            //---------------------------- END --------------------------------

            }
            break;

          case AND:{
            printf("operatore riconosciuto come AND\n");
            //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
            // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
            //              e l'operando a SX di quest'ultimo ha fallito
            if(currentProcessTable->skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable->pid = fork();
              if(currentProcessTable->pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                setenv_w(EV_STDOUTFILE,currentProcessTable->tmpOutFile);
                setenv_w(EV_STDERRFILE,currentProcessTable->tmpErrFile);
                setenv_w(EV_PINFO_OUTFILE,currentProcessTable->tmpProcInfoFile);
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_NAME,exec_argv);
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
            //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable->table = malloc(sizeof(processTable_t));
            copyTable(newTable->table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);
            //---------------------------- END --------------------------------

            }
            break;
          case OR:{

            printf("operatore riconosciuto come OR\n");
            //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
            // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
            //              e l'operando a SX di quest'ultimo ha fallito
            if(currentProcessTable->skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable->pid = fork();
              if(currentProcessTable->pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                setenv_w(EV_STDOUTFILE,currentProcessTable->tmpOutFile);
                setenv_w(EV_STDERRFILE,currentProcessTable->tmpErrFile);
                setenv_w(EV_PINFO_OUTFILE,currentProcessTable->tmpProcInfoFile);
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_NAME,exec_argv);
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
              //---------------------------- END -------------------------------

            }
            //---------------------------- END ---------------------------------

            else if(currentProcessTable->skip == TRUE){
              nextProcessTable->skip = currentProcessTable->skip || FALSE;
            }
            //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable->table = malloc(sizeof(processTable_t));
            copyTable(newTable->table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);
            //---------------------------- END --------------------------------

            }
            break;

          break;

        }
        break;

      case FILE_ :
        printf("token : <%d,%s> riconosciuto come FILE\n",currentToken->type,(char*)currentToken->value);
        break;

    }
    currentPointer += 1;
    if(currentPointer == nTokens){
      //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
                  // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
                  //              e l'operando a SX di quest'ultimo ha fallito
                  if(currentProcessTable->skip == FALSE){

                    //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
                    currentProcessTable->pid = fork();
                    if(currentProcessTable->pid == 0){

                      //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                      char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                      setenv_w(EV_STDOUTFILE,currentProcessTable->tmpOutFile);
                      setenv_w(EV_STDERRFILE,currentProcessTable->tmpErrFile);
                      setenv_w(EV_PINFO_OUTFILE,currentProcessTable->tmpProcInfoFile);
                      setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                      setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);
                      //---------------------------- END -----------------------------

                      execvp(LOGGER_EXEC_NAME,exec_argv);
                      exit_w(ERR_EXEC_FAIL);
                    }
                    else if(currentProcessTable->pid > 0){}
                    else{
                      exit_w(ERR_FORK_FAIL);
                    }
                    //---------------------------- END -------------------------------


                  }
                  //---------------------------- END ---------------------------------
                  //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
      processesList_t *newTable = malloc(sizeof(processesList_t));
      newTable->table = malloc(sizeof(processTable_t));
      copyTable(newTable->table,currentProcessTable);
      pushToTablesList(&processesListHead,&processesListTail,newTable);
      copyTable(currentProcessTable,nextProcessTable);
      clearTable(nextProcessTable);
      //---------------------------- END --------------------------------
    }
  }




//---------------------------------- ATTESA DEI FIGLI --------------------------------
  int childStatus;
  int wpid;
  while ( ( wpid = wait(&childStatus) ) > 0){
      printf("wpid = %d\n",wpid);
      printf("childStatus = %d\n",childStatus);
  }
  printf("all children are done\n");
//--------------------------------- ATTESA DEI FIGLI ---------------------------------
//-------------------------------------- END -----------------------------------------



//--------------------------- LETTURA VARIABILI D'AMBIENTE ---------------------------
  char * outLogFile;
  char * errLogFile;
  char * uniLogFile;
  outLogFile = getenv(EV_SHELL_STDOUTFILE);
  errLogFile = getenv(EV_SHELL_STDERRFILE);
  uniLogFile = getenv(EV_SHELL_UNIOUTFILE);
//--------------------------- LETTURA VARIABILI D'AMBIENTE ---------------------------
//-------------------------------------- END -----------------------------------------



//------------------------------- APERTURA FILE DI LOG -------------------------------
  int outLogFD, errLogFD, uniLogFD;
  if(outLogFile != NULL){  outLogFD = open_w(outLogFile); }
  if(errLogFile != NULL){  outLogFD = open_w(errLogFile); }
  if(uniLogFile != NULL){  outLogFD = open_w(uniLogFile); }
//------------------------------- APERTURA FILE DI LOG -------------------------------
//-------------------------------------- END -----------------------------------------



//----------------------------- LETTURA DEI LOG PARZIALI -----------------------------
//---------------------------------------- && ----------------------------------------
//------------------------------ SCRITTURA FILE DI LOG  ------------------------------
//  A questo punto del programma è necessario scorrere la lista dei processi creati.
//  Per ogni processo vanno letti i file temporanei nei quali sono state salvate le
//  informazioni relative all'esecuzione di un programma. (i.e. file di std. Output,
//  file std. Error e processInfo).
//  Ogni volta che leggo uno di questi files controllo la scelta dell'utente in merito
//  alla locazione in cui vanno salvati (e.g. salvare out ed err nello stesso file,
//  in files diversi, etc...) e dove appropriato ricopio le informazioni lette dai files
//  temporanei.


  //Finche non sono in fondo alla lista dei processi
  while(processesListTail != NULL){

    int tmpOutFD = processesListTail -> table -> tmpOutFD ;
    int tmpErrFD = processesListTail -> table -> tmpErrFD ;
    int tmpProcInfoFD = processesListTail -> table -> tmpProcInfoFD ;

    char readBuffer[CMD_OUT_BUFF_SIZE]; //Buffer per la lettura e scrittura del file
    ssize_t byteRead; //numero di Byte letti da read()
    ssize_t byteWritten; //numero di Byte scritti da write()


//---------------- SCRITTURA INFORMAZIONI DEL COMANDO ESEGUITO -----------------
    //Scrittura del nome del comando e delle opzioni utilizzate nell'invocazione
    //Controllando sempre che file sono stati definiti dall'utente per la scrittura
    int i;
    for(i=0;i<processesListTail -> table -> nOptions;i++){

      //Controllo File di output
      if(outLogFile != NULL){
        //Se è la prima opzione scrivo anche il nome del comando
        if(i==0){
          byteWritten = write_w(outLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        }
        byteWritten = write_w(outLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
      }

      //Controllo File di errore
      if(errLogFile != NULL){
        if(i==0){
          byteWritten = write_w(outLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        }
        byteWritten = write_w(outLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
      }

      //Controllo File di output ed errore unificati
      if(uniLogFile != NULL){
        if(i==0){
          byteWritten = write_w(outLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        }
        byteWritten = write_w(outLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
      }

    }


//-------------------------------------- END -------------------------------------




//------------------ LETTURA FILE TMP CONTENENTE PROC. INFO ----------------------

    //Riposiziono l'indice di lettura per il file temporaneo
    lseek_w(tmpProcInfoFD,0,SEEK_SET);
    while( (byteRead = read_w(tmpProcInfoFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
      //Controllo dove scrivere le informazioni e le scrivo
      if(outLogFile != NULL){byteWritten = write_w(outLogFD,readBuffer,byteRead);}
      if(errLogFile != NULL){byteWritten = write_w(errLogFD,readBuffer,byteRead);}
      if(uniLogFile != NULL){byteWritten = write_w(uniLogFD,readBuffer,byteRead);}
    }
    //Pulisco il buffer
    memset(readBuffer,0,CMD_OUT_BUFF_SIZE);

//-------------------------------------- END -----------------------------------




//---------------------- LETTURA FILE TMP ASSOCIATO A STD. OUT -----------------

  //Riposiziono l'indice di lettura per il file temporaneo
  lseek_w(tmpOutFD,0,SEEK_SET);
  while( (byteRead = read_w(tmpOutFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
    //Controllo dove scrivere le informazioni e le scrivo
    if(outLogFile != NULL){byteWritten = write_w(outLogFD,readBuffer,byteRead);}
    if(errLogFile != NULL){byteWritten = write_w(errLogFD,readBuffer,byteRead);}
    if(uniLogFile != NULL){byteWritten = write_w(uniLogFD,readBuffer,byteRead);}
  }
  //Pulisco il buffer
  memset(readBuffer,0,CMD_OUT_BUFF_SIZE);

//-------------------------------------- END -----------------------------------




//------------------- LETTURA FILE TMP ASSOCIATO A STD. ERR --------------------

  //Riposiziono l'indice di lettura per il file temporaneo
  lseek_w(tmpErrFD,0,SEEK_SET);
  while( (byteRead = read_w(tmpErrFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
    //Controllo dove scrivere le informazioni e le scrivo
    if(outLogFile != NULL){byteWritten = write_w(outLogFD,readBuffer,byteRead);}
    if(errLogFile != NULL){byteWritten = write_w(errLogFD,readBuffer,byteRead);}
    if(uniLogFile != NULL){byteWritten = write_w(uniLogFD,readBuffer,byteRead);}
  }
  //Pulisco il buffer
  memset(readBuffer,0,CMD_OUT_BUFF_SIZE);

//---------------------------------- END ---------------------------------------






//----------------- PULIZIA E RIMOZIONE DEI FILE TEMPORANEI --------------------
    //Eliminazione del file temporaneo di output
    unlink(processesListTail -> table -> tmpOutFile);
    //Eliminazione del file temporaneo di errore
    unlink(processesListTail -> table -> tmpErrFile);
    //Eliminazione del file temporaneo di proces Info
    unlink(processesListTail -> table -> tmpProcInfoFile);
//---------------------------------- END ---------------------------------------

//------------ CHIUSURA DEI FILE DESCRIPTORS ASSOCIATI AI FILE TMP -------------

    //Chiusura del file descriptor associato al file temporaneo proc. Info
    close(tmpProcInfoFD);
    //Chiusura del file descriptor associato al file temporaneo di output
    close(tmpOutFD);
    //Chiusura del file descriptor associato al file temporaneo di err
    close(tmpErrFD);

//---------------------------------- END ---------------------------------------

    //Leggi il prossimo elemento della lista
    processesListTail = processesListTail -> next;
  }


//------------------------------------ END -------------------------------------
//------------------------------------ && --------------------------------------
//------------------------------------ END -------------------------------------


  printTablesList(processesListHead,processesListTail);
  printPipesList(pipesHead,pipesTail);

  exit(EXIT_SUCCESS);
}
