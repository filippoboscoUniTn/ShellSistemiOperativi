/*
    Codice sorgente del controller
    Questo programma viene invocato dalla shell per ogni stringa separata dal simbolo ;
    Tale stringa rappresenta una sequenza di comandi e operandi da eseguire.
    Per ogni tale stringa il controller :
      1)  Trasforma l'input grezzo in una sequenza di token facendo contemporaneamente controlli sulla sintassi
      2)  Esecuzione di un logger per ogni comando incontrato e applicazione della logica dettata dagli operatori :
          Per fare ciò il controller utilizza due tabelle : la tabella del processo corrente (currentProcessTable, CPT) e quella del processo successivo(nextProcessTable, NPT)
          Scansionando la sequenza di token da SX verso DX il comportamento dell'algoritmo è dettato dal tipo e dal valore del token incontrato :
            a)  Se il token è un comando il nome del comando viene inserito in CPT e vengono creati dei file temporanei per la scrittura da parte del logger di informazioni sull'esecuzione del comando
            b)  Se il token rappresenta un opzione essa viene aggiunta in CPT
            c)  Se il token rappresenta un operatore, viene valutato il tipo di operatore in analisi.
                Per un operatore di tipo :
                  - INPUT REDIRECT ( < ) : Viene analizzato il token successivo per raccogliere il nome del file. Tale file viene aperto e associato
                                           come flusso di input per il processo corrente

                  - OUTPUT REDIRECT ( > ): Viene analizzato il token successivo per raccogliere il nome del file. Tale file viene aperto e associato
                                           come flusso di uscita per il processo corrente

                  - PIPE ( | )           : Viene creata una pipe, il lato WRITE viene associato al processo presente in CPT mentre il lato READ viene associato al processo successivo in NPT.
                                           Viene controllato se il processo in CPT vada eseguito. In caso affermativo viene preparato l'ambiente per l'esecuzione
                                           del logger e vengono chiuse eventuali pipe inutilizzate.

                  - AND ( && )           : Viene subito controllato se il processo in CPT vada eseguito. In caso negativo viene impostata la flag di non esecuzione del prossimo processo come il risultato dell'operatore AND booleano di false e della flag del processo corrente
                                           In caso invece il comando vada eseguito viene preparato l'ambiente per l'esecuzione del logger per il comando in CPT. Il controller si metter poi in attesa della conclusione del comando e imposta la Flag
                                           di non esecuzione del prossimo processo a FALSE se il processo ha terminato con successo a TRUE altrimenti.

                  - OR ( || )            : Comportamento analogo a sopra ma "opposto"

      3)  Attesa conclusione di tutti i figli generati nei passi precedenti. Qualora un figlio dovesse terminare il padre salva informazioni di controllo nella tabella associata al comando
      4) Una volta che tutti i figli hanno terminato il controller può andare a raccogliere le informazioni scritte dai logger nei files temporanei e riscriverle nei file specificati dall'utente.
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

  ssize_t byteRead; //numero di Byte letti da read()
  ssize_t byteWritten; //numero di Byte scritti da write()

  while( (currentPointer < nTokens) && !error ){
    //Leggo un token
    currentToken = inputTokens[currentPointer];
    //Controllo sul TIPO di token
    switch(currentToken->type){

      case COMMAND :

        if(strcmp(currentProcessTable->command,"") != 0)  { exit_w(ERR_CMD_EXSTS);}
        strcpy(currentProcessTable->command,currentToken->value);
        char tmpName[MAX_CMD_LEN];

        strcpy(tmpName,"./tmp/");
        //strcat(tmpName,(char*)currentToken->value);
        strcat(tmpName,"outFile_XXXXXX");
        tmpOutFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpOutFile,tmpName);
        close(tmpOutFD);

        strcpy(tmpName,"./tmp/");
        //strcat(tmpName,currentToken->value);
        strcat(tmpName,"errFile_XXXXXX");
        tmpErrFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpErrFile,tmpName);
        close(tmpErrFD);

        strcpy(tmpName,"./tmp/");
        //strcat(tmpName,currentToken->value);
        strcat(tmpName,"procInfoFile_XXXXXX");
        tmpProcInfoFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpProcInfoFile,tmpName);
        close(tmpProcInfoFD);

        currentProcessTable->nOptions = 0;

        break;

      case OPTION :


        if(currentProcessTable->nOptions > MAX_ARGUMENTS) {  exit_w(ERR_MAX_ARGS); }
        if( strcmp(currentProcessTable->options[currentProcessTable->nOptions],"") != 0)  { exit_w(ERR_OPT_EXSTS); }
        strcpy(currentProcessTable->options[currentProcessTable->nOptions],currentToken->value);
        currentProcessTable->nOptions += 1;

        break;

      case OPERATOR :


        switch(*((int*)(currentToken -> value))){

          case IN_REDIRECT:{


            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken -> type != FILE_){exit_w(ERR_FILE_XPCTD);}
            if(currentProcessTable -> inputPipe != -1){exit_w(ERR_INPUT_OVRITE);}
            strcpy(currentProcessTable -> inputFile,fileToken -> value);
            currentProcessTable -> inputPipe = open_w(currentProcessTable -> inputFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
            currentPointer += 1;
            }

          break;

          case OUT_REDIRECT:{


            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken -> type != FILE_){exit_w(ERR_FILE_XPCTD);}
            strcpy(currentProcessTable -> outRedirectFile,fileToken -> value);
            if(currentProcessTable -> outputPipe != -1){exit_w(ERR_INPUT_OVRITE);}
            currentProcessTable -> outputPipe = open_w(currentProcessTable -> outRedirectFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
            currentPointer += 1;
          }

          break;

          case PIPE:{

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

                if( (currentProcessTable -> inputPipe != -1) ){//Se è definita una pipe di input per il  processo e NON è definito un file di input
                  //NB il controllo sulla presenza di un input file serve per garantire che inputPipe + 1 sia il lato WRITE di una pipe
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                close( currentProcessTable -> outputPipe -1); //Chiusura lato READ della pipe di output
                while(processesListTail != NULL){
                  if(processesListTail -> table -> inputPipe != -1){
                    close(processesListTail-> table -> inputPipe);
                  }
                  if(processesListTail-> table -> outputPipe != -1){
                    close(processesListTail-> table -> outputPipe);
                  }
                  processesListTail = processesListTail -> next;
                }
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
                while(processesListTail != NULL){
                  if(processesListTail -> table -> inputPipe != -1){
                    close(processesListTail-> table -> inputPipe);
                  }
                  if(processesListTail-> table -> outputPipe != -1){
                    close(processesListTail-> table -> outputPipe);
                  }
                  processesListTail = processesListTail -> next;
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
                  if(status != 0){
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
                while(processesListTail != NULL){
                  if(processesListTail -> table -> inputPipe != -1){
                    close(processesListTail-> table -> inputPipe);
                  }
                  if(processesListTail-> table -> outputPipe != -1){
                    close(processesListTail-> table -> outputPipe);
                  }
                  processesListTail = processesListTail -> next;
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
                      while(processesListTail != NULL){
                        if(processesListTail -> table -> inputPipe != -1){
                          close(processesListTail-> table -> inputPipe);
                        }
                        if(processesListTail-> table -> outputPipe != -1){
                          close(processesListTail-> table -> outputPipe);
                        }
                        processesListTail = processesListTail -> next;
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

//---------------------------------- ATTESA DEI FIGLI --------------------------------
  int childStatus;
  int wpid;
  while ( ( wpid = wait(&childStatus) ) > 0){


      //processesList_t *tailTmp = processesListTail; //Lista temporanea per la ricerca della tabella del figlio che ha terminato
      while(processesListTail != NULL){ //Scorro la lista finche non trovo una tabella con pid uguale al pid del processo appena terminato

        if(processesListTail -> table -> pid == wpid){
          //Se sono definite pipe le chiudo
          if( processesListTail -> table -> inputPipe != -1){
            close_w(processesListTail -> table -> inputPipe);
          }
          if(processesListTail -> table -> outputPipe != -1){
            close_w(processesListTail -> table -> outputPipe);
          }
          //Salvo lo stato del processo in tabella
          processesListTail -> table -> status = WEXITSTATUS(childStatus);
        }
        processesListTail = processesListTail -> next;
      }
      rewindLinkedList(&processesListHead,&processesListTail);
  }


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
    outLogFD = open_w(outLogFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
    lseek_w(outLogFD,0,SEEK_END);
   }

  if(errLogFile != NULL){
     errLogFD = open_w(errLogFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
     lseek_w(errLogFD,0,SEEK_END);
    }

  if(uniLogFile != NULL){
      uniLogFD = open_w(uniLogFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
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

  if(outLogFile != NULL){
    //Se è la prima opzione scrivo anche il nome del comando
      byteWritten = write_w(outLogFD,FRM_EXP_SEPARATOR,strlen(FRM_EXP_SEPARATOR));
      byteWritten = write_w(outLogFD,FRM_INPUT_STRING,strlen(FRM_INPUT_STRING));
      byteWritten = write_w(outLogFD,rawInput,strlen(rawInput));
      byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));

      //

  }
  if(errLogFile != NULL){
    //Se è la prima opzione scrivo anche il nome del comando
    byteWritten = write_w(errLogFD,FRM_EXP_SEPARATOR,strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(errLogFD,FRM_INPUT_STRING,strlen(FRM_INPUT_STRING));
    byteWritten = write_w(errLogFD,rawInput,strlen(rawInput));
    byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));

  }
  if(uniLogFile != NULL){
    //Se è la prima opzione scrivo anche il nome del comando
    byteWritten = write_w(uniLogFD,FRM_EXP_SEPARATOR,strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(uniLogFD,FRM_INPUT_STRING,strlen(FRM_INPUT_STRING));
    byteWritten = write_w(uniLogFD,rawInput,strlen(rawInput));
    byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));

  }


  //Finche non sono in fondo alla lista dei processi
  while(processesListTail != NULL){

    int tmpOutFD = open_w(processesListTail -> table -> tmpOutFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP) ;
    int tmpErrFD = open_w(processesListTail -> table -> tmpErrFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP) ;
    int tmpProcInfoFD = open_w(processesListTail -> table -> tmpProcInfoFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP) ;



    char readBuffer[CMD_OUT_BUFF_SIZE]; //Buffer per la lettura e scrittura del file
    int totWrittenOut = 0;
    int totWrittenErr = 0;
    int totWrittenUni = 0;


//---------------- SCRITTURA INFORMAZIONI DEL COMANDO ESEGUITO -----------------
    //Scrittura del nome del comando e delle opzioni utilizzate nell'invocazione
    //Controllando sempre che file sono stati definiti dall'utente per la scrittura

    if(outLogFile != NULL){
      //Se è la prima opzione scrivo anche il nome del comando
        byteWritten = write_w(outLogFD,FRM_COMMAND,strlen(FRM_COMMAND));
        byteWritten = write_w(outLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        byteWritten = write_w(outLogFD,"\n",strlen("\n"));
        byteWritten = write_w(outLogFD,FRM_OPTIONS,strlen(FRM_OPTIONS));

        //

    }
    if(errLogFile != NULL){
      //Se è la prima opzione scrivo anche il nome del comando
      byteWritten = write_w(errLogFD,FRM_COMMAND,strlen(FRM_COMMAND));
      byteWritten = write_w(errLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
      byteWritten = write_w(errLogFD,"\n",strlen("\n"));
      byteWritten = write_w(errLogFD,FRM_OPTIONS,strlen(FRM_OPTIONS));

    }
    if(uniLogFile != NULL){
      //Se è la prima opzione scrivo anche il nome del comando
      byteWritten = write_w(uniLogFD,FRM_COMMAND,strlen(FRM_COMMAND));
      byteWritten = write_w(uniLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
      byteWritten = write_w(uniLogFD,"\n",strlen("\n"));
      byteWritten = write_w(uniLogFD,FRM_OPTIONS,strlen(FRM_OPTIONS));

    }


    int i;
    for(i=0;i<processesListTail -> table -> nOptions;i++){
      //Controllo File di output
      if(outLogFile != NULL){
        byteWritten = write_w(outLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
        byteWritten = write_w(outLogFD," ",strlen(" "));

      }

      //Controllo File di errore
      if(errLogFile != NULL){
        byteWritten = write_w(errLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
        byteWritten = write_w(errLogFD," ",strlen(" "));
      }

      //Controllo File di output ed errore unificati
      if(uniLogFile != NULL){
        byteWritten = write_w(uniLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
        byteWritten = write_w(uniLogFD," ",strlen(" "));
      }

    }


    if(outLogFile != NULL){
      byteWritten = write_w(outLogFD, "\n", strlen("\n"));
      byteWritten = write_w(outLogFD,FRM_FLD_SEPARATOR, strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(outLogFD,FRM_PROC_INFO, strlen(FRM_PROC_INFO));
      byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));
    }

    if(errLogFile != NULL){
      byteWritten = write_w(errLogFD, "\n", strlen("\n"));
      byteWritten = write_w(errLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(errLogFD,FRM_PROC_INFO, strlen(FRM_PROC_INFO));
      byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));
    }

    if(uniLogFile != NULL){
      byteWritten = write_w(uniLogFD, "\n", strlen("\n"));
      byteWritten = write_w(uniLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(uniLogFD,FRM_PROC_INFO, strlen(FRM_PROC_INFO));
      byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
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

if(outLogFile != NULL){
  byteWritten = write_w(outLogFD,FRM_FLD_SEPARATOR, strlen(FRM_FLD_SEPARATOR));
  byteWritten = write_w(outLogFD,FRM_PROC_OUT, strlen(FRM_PROC_OUT));
  byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));
}

if(errLogFile != NULL){
  byteWritten = write_w(errLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
  byteWritten = write_w(errLogFD,FRM_PROC_ERR, strlen(FRM_PROC_ERR));
  byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));
}

if(uniLogFile != NULL){
  byteWritten = write_w(uniLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
  byteWritten = write_w(uniLogFD,FRM_PROC_OUT, strlen(FRM_PROC_OUT));
  byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
}


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

if(outLogFile != NULL){
  byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));
}

if(uniLogFile != NULL){
  byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
  byteWritten = write_w(uniLogFD,FRM_PROC_ERR,strlen(FRM_PROC_ERR));
  byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
}


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

if(outLogFile != NULL){
  //manca per un motivo :)
  byteWritten = write_w(outLogFD,FRM_CMD_SEPARATOR, strlen(FRM_CMD_SEPARATOR));
  byteWritten = write_w(outLogFD,"\n",strlen("\n"));
}

if(errLogFile != NULL){
  byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));
  byteWritten = write_w(errLogFD,FRM_CMD_SEPARATOR, strlen(FRM_CMD_SEPARATOR));
  byteWritten = write_w(errLogFD,"\n",strlen("\n"));
}

if(uniLogFile != NULL){
  byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
  byteWritten = write_w(uniLogFD,FRM_CMD_SEPARATOR, strlen(FRM_CMD_SEPARATOR));
  byteWritten = write_w(uniLogFD,"\n",strlen("\n"));
}




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

  if(outLogFile != NULL){
    byteWritten = write_w(outLogFD,FRM_EXP_SEPARATOR, strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(outLogFD,"\n",strlen("\n"));
  }

  if(errLogFile != NULL){
    byteWritten = write_w(errLogFD,FRM_EXP_SEPARATOR, strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(errLogFD,"\n",strlen("\n"));
  }

  if(uniLogFile != NULL){
    byteWritten = write_w(uniLogFD,FRM_EXP_SEPARATOR, strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(uniLogFD,"\n",strlen("\n"));
  }

  //Rewind della tail per farla tornare a puntare al primo elemento della lista!
  rewindLinkedList(&processesListHead,&processesListTail);
//------------------------------------ END -------------------------------------
//------------------------------------ && --------------------------------------
//------------------------------------ END -------------------------------------


  exit(EXIT_SUCCESS);
}
