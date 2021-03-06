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

  int tmpOutFD;
  int tmpErrFD;
  int tmpProcInfoFD;

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

        strcpy(tmpName,"./tmp/");
        strcat(tmpName,(char*)currentToken->value);
        strcat(tmpName,"outFile_XXXXXX");
        tmpOutFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpOutFile,tmpName);
        close(tmpOutFD);

        strcpy(tmpName,"./tmp/");
        strcat(tmpName,currentToken->value);
        strcat(tmpName,"errFile_XXXXXX");
        tmpErrFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpErrFile,tmpName);
        close(tmpErrFD);

        strcpy(tmpName,"./tmp/");
        strcat(tmpName,currentToken->value);
        strcat(tmpName,"procInfoFile_XXXXXX");
        tmpProcInfoFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpProcInfoFile,tmpName);
        close(tmpProcInfoFD);

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

        printf("token : <%d,%d> riconosciuto come OPERATORE\n",currentToken->type,*((int*)(currentToken -> value)));
        switch(*((int*)(currentToken -> value))){

          case IN_REDIRECT:{

            printf("operatore riconosciuto come IN_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken -> type != FILE_){exit_w(ERR_FILE_XPCTD);}
            if(currentProcessTable -> inputPipe != -1){exit_w(ERR_INPUT_OVRITE);}
            strcpy(currentProcessTable -> inputFile,fileToken -> value);
            currentProcessTable -> inputPipe = open_w(currentProcessTable -> inputFile);
            currentPointer += 1;
            }

          break;

          case OUT_REDIRECT:{

            printf("operatore riconosciuto come OUT_REDIRECT\n");
            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken -> type != FILE_){exit_w(ERR_FILE_XPCTD);}
            strcpy(currentProcessTable -> outRedirectFile,fileToken -> value);
            currentPointer += 1;
          }

          break;

          case PIPE:{
            printf("operatore riconosciuto come PIPE\n");
            //---------------------- ALLOCAZIONE DELLA PIPE --------------------
            //--- PER L'IPC DEGLI OPERANDI A DX E SX DELL OPERATORE DI PIPE ----
            pipesList_t *newPipe = malloc(sizeof(pipesList_t));
            pipe_w(newPipe -> pipe);
            currentProcessTable -> outputPipe = newPipe -> pipe[WRITE];
            nextProcessTable -> inputPipe = newPipe -> pipe[READ];
            pushToPipesList(&pipesHead,&pipesTail,newPipe);
            //---------------------------- END ---------------------------------



            //------------ CONTROLLO SE IL COMANDO VA ESEGUITO -----------------
            // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
            //              e l'operando a SX di quest'ultimo ha fallito
            if(currentProcessTable -> skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable -> pid = fork();
              if(currentProcessTable -> pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                //-------------------------- & ---------------------------------
                //---------- CHIUSURA DEL LATO READ DELLA PIPE -----------------
                //----------------- SU CUI IL PROCESSO SCRIVE ------------------
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                setenv_w(EV_STDOUTFILE,currentProcessTable -> tmpOutFile);
                setenv_w(EV_STDERRFILE,currentProcessTable -> tmpErrFile);
                setenv_w(EV_PINFO_OUTFILE,currentProcessTable -> tmpProcInfoFile);
                setenv_wi(EV_PIPE_IN,currentProcessTable -> inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable -> outputPipe);
                close(currentProcessTable -> outputPipe -1);//Chiusura lato READ della pipe su cui il processo scrive

                if( (currentProcessTable -> inputPipe != -1) && (strcmp(currentProcessTable -> inputFile, "") == 0) ){//Se è definita una pipe di input per il  processo e NON è definito un file di input
                  //NB il controllo sulla presenza di un input file serve per garantire che inputPipe + 1 sia il lato WRITE di una pipe
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                close( currentProcessTable -> outputPipe -1); //Chiusura lato READ della pipe di output
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_PATH,exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable -> pid < 0){exit_w(ERR_FORK_FAIL);}
              //---------------------------- END -------------------------------


            }
            //---------------------------- END ---------------------------------



            //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable -> table = malloc(sizeof(processTable_t));
            copyTable(newTable -> table,currentProcessTable);
            pushToTablesList(&processesListHead,&processesListTail,newTable);
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);
            //---------------------------- END ---------------------------------

            }
            break;

          case AND:{
            printf("operatore riconosciuto come AND\n");
            //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
            // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
            //              e l'operando a SX di quest'ultimo ha fallito
            if(currentProcessTable -> skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable -> pid = fork();
              if(currentProcessTable -> pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                //------------------------------- & ----------------------------
                //----- CHIUSURA LATO WRITE NEL CASO SIA DEFINITA UNA PIPE DI INPUT
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                setenv_w(EV_STDOUTFILE,currentProcessTable->tmpOutFile);
                setenv_w(EV_STDERRFILE,currentProcessTable->tmpErrFile);
                setenv_w(EV_PINFO_OUTFILE,currentProcessTable->tmpProcInfoFile);
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);

                if(currentProcessTable -> inputPipe != -1){//Se è definita una pipe di input per il  processo
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_PATH,exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable -> pid > 0){
                int wstatus;
                waitpid_w(currentProcessTable -> pid,&wstatus,0);
                if(WIFEXITED(wstatus)){
                  int status = WEXITSTATUS(wstatus);
                  if(status < 0){
                    nextProcessTable->skip = TRUE;
                  }
                  currentProcessTable -> status = wstatus;
                }
              }
              else{
                exit_w(ERR_FORK_FAIL);
              }
            }
            else if(currentProcessTable -> skip == TRUE){
              nextProcessTable -> skip = currentProcessTable -> skip && FALSE;
            }
            //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable -> table = malloc(sizeof(processTable_t));
            copyTable(newTable -> table,currentProcessTable);
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
            if(currentProcessTable -> skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable -> pid = fork();
              if(currentProcessTable -> pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                //------------------------------- & ----------------------------
                //----- CHIUSURA LATO WRITE NEL CASO SIA DEFINITA UNA PIPE DI INPUT
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                setenv_w(EV_STDOUTFILE,currentProcessTable->tmpOutFile);
                setenv_w(EV_STDERRFILE,currentProcessTable->tmpErrFile);
                setenv_w(EV_PINFO_OUTFILE,currentProcessTable->tmpProcInfoFile);
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);

                if(currentProcessTable -> inputPipe != -1){//Se è definita una pipe di input per il  processo
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_PATH,exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable -> pid > 0){
                int wstatus;
                waitpid_w(currentProcessTable -> pid,&wstatus,0);
                if(WIFEXITED(wstatus)){
                  int status = WEXITSTATUS(wstatus);
                  if(status == 0){
                    nextProcessTable -> skip = TRUE;
                  }
                  currentProcessTable -> status = wstatus;
                }
              }
              else{
                exit_w(ERR_FORK_FAIL);
              }
              //---------------------------- END -------------------------------

            }
            //---------------------------- END ---------------------------------

            else if(currentProcessTable -> skip == TRUE){
              nextProcessTable -> skip = currentProcessTable -> skip || FALSE;
            }
            //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
            processesList_t *newTable = malloc(sizeof(processesList_t));
            newTable -> table = malloc(sizeof(processTable_t));
            copyTable(newTable -> table,currentProcessTable);
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
                  if(currentProcessTable -> skip == FALSE){

                    //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
                    currentProcessTable -> pid = fork();
                    if(currentProcessTable -> pid == 0){

                      //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                      //------------------------------- & ----------------------------
                      //----- CHIUSURA LATO WRITE NEL CASO SIA DEFINITA UNA PIPE DI INPUT
                      char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                      setenv_w(EV_STDOUTFILE,currentProcessTable -> tmpOutFile);
                      setenv_w(EV_STDERRFILE,currentProcessTable -> tmpErrFile);
                      setenv_w(EV_PINFO_OUTFILE,currentProcessTable -> tmpProcInfoFile);
                      setenv_wi(EV_PIPE_IN,currentProcessTable -> inputPipe);
                      setenv_wi(EV_PIPE_OUT,currentProcessTable -> outputPipe);

                      if(currentProcessTable -> inputPipe != -1 && strcmp(currentProcessTable -> inputFile,"") == 0){//Se è definita una pipe di input per il  processo e NON è definito un file di input
                        //NB il controllo sulla presenza di un input file serve per garantire che inputPipe + 1 sia il lato WRITE di una pipe
                        close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                      }
                      //---------------------------- END -----------------------------

                      execvp(LOGGER_EXEC_PATH,exec_argv);
                      exit_w(ERR_EXEC_FAIL);
                    }
                    else if(currentProcessTable -> pid > 0){}
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

    printTablesList(processesListHead,processesListTail);
    printPipesList(pipesHead,pipesTail);
    rewindLinkedList(&processesListHead,&processesListTail);



//---------------------------------- ATTESA DEI FIGLI --------------------------------
  int childStatus;
  int wpid;
  while ( ( wpid = wait(&childStatus) ) > 0){
      printf("wpid = %d\n",wpid);
      printf("childStatus = %d\n",childStatus);
      processesList_t *tailTmp = processesListTail; //Lista temporanea per la ricerca della tabella del figlio che ha terminato
      while(tailTmp != NULL){ //Scorro la lista finche non trovo una tabella con pid uguale al pid del processo appena terminato

        if(tailTmp -> table -> pid == wpid){
          //Se sono definite pipe le chiudo
          if( tailTmp -> table -> inputPipe != -1){
            close_w(tailTmp -> table -> inputPipe);
          }
          if(tailTmp -> table -> outputPipe != -1){
            close_w(tailTmp -> table -> outputPipe);
          }
          //Salvo lo stato del processo in tabella
          tailTmp -> table -> status = childStatus;
        }
        tailTmp = tailTmp -> next;
      }
  }
  rewindLinkedList(&processesListHead,&processesListTail);

  printf("all children are done\n");
//--------------------------------- ATTESA DEI FIGLI ---------------------------------
//-------------------------------------- END -----------------------------------------



//----------------------- LETTURA VARIABILI D'AMBIENTE -------------------------
//-------------- CONTENENTI INFORMAZIONI SUI FILE E LE MODALITÀ DI LOG ---------
  char * outLogFile;
  char * errLogFile;
  char * uniLogFile;
  char * maxOutputLength;
  int maxOutput;

  outLogFile = getenv(EV_SHELL_STDOUTFILE);
  errLogFile = getenv(EV_SHELL_STDERRFILE);
  uniLogFile = getenv(EV_SHELL_UNIOUTFILE);
  maxOutputLength = getenv(EV_MAXLEN);
  printf("outLogFile = %s\n",outLogFile);
  printf("errLogFile = %s\n",errLogFile);
  printf("uniLogFile = %s\n",uniLogFile);
  printf("maxOutputLength = %s\n",maxOutputLength);
  if(maxOutputLength != NULL){
    maxOutput = atoi(maxOutputLength);
  }
  else{
    maxOutput = -1;
  }
//--------------------------- LETTURA VARIABILI D'AMBIENTE ---------------------------
//-------------------------------------- END -----------------------------------------



//------------------------------- APERTURA FILE DI LOG -------------------------------
//--------------------- E RIPOSIZIONAMENTO CURSORE -----------------------
  int outLogFD, errLogFD, uniLogFD;
  if(outLogFile != NULL){
    outLogFD = open_w(outLogFile);
    lseek_w(outLogFD,0,SEEK_END);
   }

  if(errLogFile != NULL){
     errLogFD = open_w(errLogFile);
     lseek_w(errLogFD,0,SEEK_END);
    }

  if(uniLogFile != NULL){
      uniLogFD = open_w(uniLogFile);
      lseek_w(uniLogFD,0,SEEK_END);
    }
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

    int tmpOutFD = open_w(processesListTail -> table -> tmpOutFile) ;
    int tmpErrFD = open_w(processesListTail -> table -> tmpErrFile) ;
    int tmpProcInfoFD = open_w(processesListTail -> table -> tmpProcInfoFile) ;
    printf("tmpOutFD = %d\n",tmpOutFD );
    printf("tmpErrFD = %d\n",tmpErrFD );
    printf("tmpProcInfoFD = %d\n",tmpProcInfoFD );
    char readBuffer[CMD_OUT_BUFF_SIZE]; //Buffer per la lettura e scrittura del file
    ssize_t byteRead; //numero di Byte letti da read()
    ssize_t byteWritten; //numero di Byte scritti da write()
    int totWrittenOut = 0;
    int totWrittenErr = 0;
    int totWrittenUni = 0;


//---------------- SCRITTURA INFORMAZIONI DEL COMANDO ESEGUITO -----------------
    //Scrittura del nome del comando e delle opzioni utilizzate nell'invocazione
    //Controllando sempre che file sono stati definiti dall'utente per la scrittura
    if(outLogFile != NULL){
      //Se è la prima opzione scrivo anche il nome del comando
        byteWritten = write_w(outLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        printf("byteWritten = %ld to out\n",byteWritten);

    }
    if(errLogFile != NULL){
      //Se è la prima opzione scrivo anche il nome del comando
        byteWritten = write_w(errLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        printf("byteWritten = %ld to err\n",byteWritten);

    }
    if(uniLogFile != NULL){
      //Se è la prima opzione scrivo anche il nome del comando
        byteWritten = write_w(uniLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        printf("byteWritten = %ld to uni\n",byteWritten);

    }


    int i;
    for(i=0;i<processesListTail -> table -> nOptions;i++){

      //Controllo File di output
      if(outLogFile != NULL){
        byteWritten = write_w(outLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
      }

      //Controllo File di errore
      if(errLogFile != NULL){
        byteWritten = write_w(errLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
      }

      //Controllo File di output ed errore unificati
      if(uniLogFile != NULL){
        byteWritten = write_w(uniLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
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
    if(outLogFile != NULL){
      if(maxOutput == -1){
        byteWritten = write_w(outLogFD,readBuffer,byteRead);
      }
      else if(maxOutput != -1 && totWrittenOut < maxOutput){
        int writableOut = maxOutput - totWrittenOut;
        if(writableOut >= byteRead){
          byteWritten = write_w(outLogFD,readBuffer,byteRead);
          totWrittenOut += byteWritten;
        }
        else{
          byteWritten = write_w(outLogFD,readBuffer,writableOut);
          totWrittenOut += byteWritten;
        }
      }
    }
    if(uniLogFile != NULL){
      if(maxOutput == -1){
        byteWritten = write_w(uniLogFD,readBuffer,byteRead);
      }
      else if(maxOutput != -1 && totWrittenUni < maxOutput){
        int writableUni = maxOutput - totWrittenUni;
        if(writableUni >= byteRead){
          byteWritten = write_w(uniLogFD,readBuffer,byteRead);
          totWrittenUni += byteWritten;
        }
        else{
          byteWritten = write_w(uniLogFD,readBuffer,writableUni);
          totWrittenUni += byteWritten;
        }
      }
    }
  }
  //Pulisco il buffer
  memset(readBuffer,0,CMD_OUT_BUFF_SIZE);

//-------------------------------------- END -----------------------------------




//------------------- LETTURA FILE TMP ASSOCIATO A STD. ERR --------------------

  //Riposiziono l'indice di lettura per il file temporaneo
  lseek_w(tmpErrFD,0,SEEK_SET);
  while( (byteRead = read_w(tmpErrFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
    //Controllo dove scrivere le informazioni e le scrivo
    if(errLogFile != NULL){

      if(maxOutput == -1){
        byteWritten = write_w(errLogFD,readBuffer,byteRead);
      }
      else if(maxOutput != -1 && totWrittenErr < maxOutput){

        int writableErr = maxOutput - totWrittenErr;
        if(writableErr >= byteRead){
          byteWritten = write_w(errLogFD,readBuffer,byteRead);
          totWrittenErr += byteWritten;
        }
        else{
          byteWritten = write_w(errLogFD,readBuffer,writableErr);
          totWrittenErr += byteWritten;
        }

      }

    }
    if(uniLogFile != NULL){

      if(maxOutput == -1){
        byteWritten = write_w(uniLogFD,readBuffer,byteRead);
      }
      else if(maxOutput != -1 && totWrittenUni < maxOutput){

        int writableUni = maxOutput - totWrittenUni;
        if(writableUni >= byteRead){
          byteWritten = write_w(uniLogFD,readBuffer,byteRead);
          totWrittenUni += byteWritten;
        }
        else{
          byteWritten = write_w(uniLogFD,readBuffer,writableUni);
          totWrittenUni += byteWritten;
        }

      }

    }
  }
  //Pulisco il buffer
  memset(readBuffer,0,CMD_OUT_BUFF_SIZE);

//---------------------------------- END ---------------------------------------






//----------------- PULIZIA E RIMOZIONE DEI FILE TEMPORANEI --------------------
    // //Eliminazione del file temporaneo di output
    // unlink(processesListTail -> table -> tmpOutFile);
    // //Eliminazione del file temporaneo di errore
    // unlink(processesListTail -> table -> tmpErrFile);
    // //Eliminazione del file temporaneo di proces Info
    // unlink(processesListTail -> table -> tmpProcInfoFile);
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

  //Rewind della tail per farla tornare a puntare al primo elemento della lista!
  rewindLinkedList(&processesListHead,&processesListTail);
//------------------------------------ END -------------------------------------
//------------------------------------ && --------------------------------------
//------------------------------------ END -------------------------------------

  printTablesList(processesListHead,processesListTail);
  printPipesList(pipesHead,pipesTail);

  exit(EXIT_SUCCESS);
}
