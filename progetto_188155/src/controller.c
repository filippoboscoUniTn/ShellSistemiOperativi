/*
    Codice sorgente del controller
    Questo programma viene invocato dalla shell per ogni stringa separata dal simbolo ;
    Tale stringa rappresenta una sequenza di comandi e operandi da eseguire.
    Per ogni tale stringa il controller :

      1)  Trasforma l'input grezzo in una sequenza di token facendo contemporaneamente alcuni controlli sulla sintassi

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

                  - OR ( || )           : Comportamento opposto al precedente

          L'algoritmo continua a scorrere i token finchè non arriva a all'ultimo token e verifica se esso comporti l'esecuzione di un ulteriore logger.

      3)  Attesa conclusione di tutti i figli generati nei passi precedenti. Qualora un figlio dovesse terminare il padre salva informazioni di controllo nella tabella associata al comando
      4) Una volta che tutti i figli hanno terminato il controller può andare a raccogliere le informazioni scritte dai logger nei files temporanei e riscriverle nei file specificati dall'utente.
      //TODO : Per una gestione migliore l'implementazione sopra andrebbe sostituita con un handler per il segnale SIGCHLD.
               Ogni volta che un logger termina il controller gestisce il segnale, loggando le informazioni e i risultati del processo. A seguito va fatta pulizia e rilascio delle risorse
*/

//------------------------- EXTERNAL LIBRARIES INCLUSION -----------------------
#include "libs/std_libraries.h"
#include "libs/macros.h"
#include "libs/types.h"
#include "libs/functions.h"
//-------------------------------------- END -----------------------------------



//------------------------- GLOBAL VARIABLES DEC. & INIT -----------------------
processesList_t *globalHead,*globalTail;
 sig_atomic_t WAITING = FALSE;
 int W8PID = -1; //structure where the handler saves process' informations (PID, UID, and so on)
 int W8STATUS = -1;
//-------------------------------------- END -----------------------------------

//-------------------------- SIGCHLD HANDLER -----------------------------------
//customized SIGCHLD handler
void sigchld_handler(int signum, siginfo_t *info, void *ucontext){
  printf("sigchld_handler invoked pid = %d\n",info -> si_pid);
  printf("W8PID == %d\n",W8PID);
  printf("WAITING == %d\n",WAITING );
  while(globalTail){
    //Scorro la lista finchè non trovo la tabella associata al processo terminato
    if(globalTail -> table -> pid == info -> si_pid){
      //Se sono definite pipe le chiudo
      if( globalTail -> table -> inputPipe != 1){
        printf("closing input pipe\n");
        close_w(globalTail -> table -> inputPipe);
        globalTail -> table -> inputPipe = 1;

      }
      if(globalTail -> table -> outputPipe != 2){
        printf("closing output pipe\n");
        close_w(globalTail -> table -> outputPipe);
        globalTail -> table -> outputPipe = 2;
      }
      printf("saving status : %d\n",info -> si_status );
      //Salvo lo stato del processo in tabella
      globalTail -> table -> status = info -> si_status;
      if(W8PID == info -> si_pid){
        printf("waitpid = pid\n");
        WAITING = FALSE;
        W8PID = -1;
        W8STATUS = info -> si_status;
      }
    }

    globalTail = globalTail -> next;
  }
  rewindLinkedList(&globalHead,&globalTail);
	return;
}
//------------------------------------------------------------------------------





//---------------------------- PARAMETERS DEFINITION ---------------------------
//argv[0] -> EXECUTABLE NAME
//argv[1] -> RAW INPUT
//argv[>1] -> UNDEFINED -> ERROR
//-------------------------------------- END -----------------------------------
int main(int argc,char **argv){


//------------------ ARGUMENTS CHEKING FOR CORRECT PROGRAM'S INVOCATION --------
  if(argc != 2){
    exit_w(ERR_INVCATION_CNT);
  }
//-------------------------------------- END -----------------------------------



//-------------------------------- SIGCHLD HANDLER REPLACEMENT -----------------
struct sigaction act; //structure to be passed to the sigaction
act.sa_flags = SA_SIGINFO; //tells the sigaction that we want to execute sigaction and not sighandler
sigemptyset(&act.sa_mask); //initialize the bitmask
act.sa_sigaction = sigchld_handler; //tells the sigaction to execute our handler
sigaction(SIGCHLD, &act, NULL); //tells the OS to execute our handler when receiving SIGCHLD
//-------------------------------------- END -----------------------------------



//--------------------------- VARIABLES DECLARATION ----------------------------

//variabile di ciclo
int i;

//Numero di tokens
int nTokens;

//Stringa per contenere  l'espressione ricevuta dalla shell
char rawInput[MAX_CMD_LEN];

//puntatore alla lista dei token
token_t **inputTokens;

//Puntatore al token in analisi
token_t *currentToken;

//Puntatore all'indice del token corrente
int currentPointer;

//Tabelle dei processi
processTable_t *currentProcessTable,*nextProcessTable;

//Lista delle pipe
pipesList_t *pipesHead,*pipesTail;

//Lista delle tabelle dei processi
processesList_t *processesListHead,*processesListTail;


//intero per memorizzare lo stato di terminazione di un figlio
int childStatus;

//intero per memorizzare il pid del figlio che ha terminato
int wpid;

//File descriptor associato ad un file temporaneo generato dal logger
int tmpOutFD;
int tmpErrFD;
int tmpProcInfoFD;

//numero di Byte letti da read() e da write()
ssize_t byteRead;
ssize_t byteWritten;

//Nome dei file di log specificati dall'utente
char * outLogFile;
char * errLogFile;
char * uniLogFile;
char * writeProcInfo;

//Interi per il file descriptor associato ai file di log aperti dal controller
int outLogFD;
int errLogFD;
int uniLogFD;

//Numero massimo di caratteri scritti nei file di logs
char * maxOutputLength;
int maxOutput;

//Buffer per la lettura e scrittura del file
char readBuffer[CMD_OUT_BUFF_SIZE];

//totale scritto per un dato file (utilizzato per tenere traccia di quanto vada scritto se MAX_LEN è stata settata dall'utente)
int totWrittenOut;
int totWrittenErr;
int totWrittenUni;


//--------------------------- VARIABLES DECLARATION ----------------------------
//-------------------------------------- END -----------------------------------



//------------------------- VARIABLES INITIALIZATION ---------------------------

//@Start
nTokens = 0;

//Copia del input da shell nel buffer
strcpy(rawInput,argv[1]);

//@Start
currentPointer = 0;

//Inizializzazione delle liste a NULL
pipesHead = NULL;
pipesTail = NULL;
processesListHead = NULL;
processesListTail = NULL;
globalHead = NULL;
globalTail = NULL;
//Inizializzazione delle tabelle dei processi CPT e NPT
  //CPT
  currentProcessTable = malloc(sizeof(processTable_t));
  clearTable(currentProcessTable);

  //NPT
  nextProcessTable = malloc(sizeof(processTable_t));
  clearTable(nextProcessTable);



  //----------------------- LETTURA VARIABILI D'AMBIENTE -----------------------
  outLogFile = getenv(EV_SHELL_STDOUTFILE);
  errLogFile = getenv(EV_SHELL_STDERRFILE);
  uniLogFile = getenv(EV_SHELL_UNIOUTFILE);

  writeProcInfo = getenv(EV_PROC_INFO); //Se è definita vanno scritte anche le info del processo
  maxOutputLength = getenv(EV_MAXLEN);

  if(maxOutputLength != NULL){
    maxOutput = atoi(maxOutputLength);
  }
  else{
    maxOutput = -1;
  }
  //-------------------------------------- END ---------------------------------



//------------------------- VARIABLES INITIALIZATION ---------------------------
//-------------------------------------- END -----------------------------------





  //------------------------------ RAW INPUT TOKENIZATION ----------------------
  inputTokens = tokenize(rawInput,&nTokens);
  //-------------------------------------- END ---------------------------------






  //------------------- ALGORITMO DI GENERAZIONE DEI LOGGER --------------------
  while( currentPointer < nTokens ){

    //Lettura di un token
    currentToken = inputTokens[currentPointer];

    //----------------------- CONTROLLO SUL TIPO DI TOKEN ----------------------
    switch(currentToken->type){

      //------------------------------- CASE COMMAND ---------------------------------
      case COMMAND :

        //------------------ INSERIMENTO DEL COMANDO IN CPT --------------------
        if(strcmp(currentProcessTable->command,"") != 0)  { exit_w(ERR_CMD_EXSTS);}
        strcpy(currentProcessTable->command,currentToken->value);
        char tmpName[MAX_CMD_LEN];
        //------------------------------ END -----------------------------------



        //-------------------- CREAZIONE FILE TEMPORANEI -----------------------
        if(outLogFile || uniLogFile){
          strcpy(tmpName,"tmp/");
          strcat(tmpName,"outFile_XXXXXX");
          tmpOutFD = mkstemp_w(tmpName);
          strcpy(currentProcessTable->tmpOutFile,tmpName);
          close(tmpOutFD);
        }
        if(errLogFile  || uniLogFile){
          strcpy(tmpName,"tmp/");
          strcat(tmpName,"errFile_XXXXXX");
          tmpErrFD = mkstemp_w(tmpName);
          strcpy(currentProcessTable->tmpErrFile,tmpName);
          close(tmpErrFD);
        }
        if(!writeProcInfo){
        strcpy(tmpName,"tmp/");
        strcat(tmpName,"procInfoFile_XXXXXX");
        tmpProcInfoFD = mkstemp_w(tmpName);
        strcpy(currentProcessTable->tmpProcInfoFile,tmpName);
        close(tmpProcInfoFD);
      }
        //----------------------------- END ------------------------------------



        //--------------- INIZIALIZZAZIONE DEL NUMERO DI OPZIONI----------------
        currentProcessTable->nOptions = 0;
        //----------------------------- END ------------------------------------

        break;
      //--------------------------- CASE COMMAND END ---------------------------------



      //------------------------------- CASE OPTION ----------------------------------
      case OPTION :

        //------------- COPIA DELL'OPZIONE NELLA LISTA DELLE OPZIONI -----------
        if(currentProcessTable->nOptions > MAX_ARGUMENTS) {  exit_w(ERR_MAX_ARGS); }
        if( strcmp(currentProcessTable->options[currentProcessTable->nOptions],"") != 0)  { exit_w(ERR_OPT_EXSTS); }
        strcpy(currentProcessTable->options[currentProcessTable->nOptions],currentToken->value);
        currentProcessTable->nOptions += 1;
        //----------------------------- END ------------------------------------

        break;
      //---------------------------- CASE OPTION END --------------------------------



      //------------------------------- CASE OPERATOR --------------------------------
      case OPERATOR :


        switch(*((int*)(currentToken -> value))){

          case IN_REDIRECT:{

              int nextPointer = currentPointer +1;
              token_t *fileToken = inputTokens[nextPointer];
              if(fileToken -> type != REDIR_FILE){exit_w( ERR_REDIR_FILEXPCTD);}
              if(currentProcessTable -> inputPipe != 1){exit_w(ERR_INPUT_OVRITE);}
              strcpy(currentProcessTable -> inputFile,fileToken -> value);
              currentProcessTable -> inputPipe = open_w(currentProcessTable -> inputFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
              currentPointer += 1;
            }
          break;

          case OUT_REDIRECT:{

            int nextPointer = currentPointer +1;
            token_t *fileToken = inputTokens[nextPointer];
            if(fileToken -> type != REDIR_FILE){exit_w( ERR_REDIR_FILEXPCTD);}
            strcpy(currentProcessTable -> outRedirectFile,fileToken -> value);
            if(currentProcessTable -> outputPipe != 2){exit_w(ERR_INPUT_OVRITE);}
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
                if(outLogFile || uniLogFile){setenv_w(EV_STDOUTFILE,currentProcessTable -> tmpOutFile);}
                if(errLogFile  || uniLogFile){setenv_w(EV_STDERRFILE,currentProcessTable -> tmpErrFile);}
                if(!writeProcInfo){setenv_w(EV_PINFO_OUTFILE,currentProcessTable -> tmpProcInfoFile);}
                setenv_wi(EV_PIPE_IN,currentProcessTable -> inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable -> outputPipe);

                if( (currentProcessTable -> inputPipe != 1) ){//Se è definita una pipe di input per il  processo e NON è definito un file di input
                  //NB il controllo sulla presenza di un input file serve per garantire che inputPipe + 1 sia il lato WRITE di una pipe
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                close( currentProcessTable -> outputPipe -1); //Chiusura lato READ della pipe di output

                while(processesListTail != NULL){ //Chiusura delle pipe associate agli altri operandi
                  if(processesListTail -> table -> inputPipe != 1){
                    close(processesListTail-> table -> inputPipe);
                  }
                  if(processesListTail-> table -> outputPipe != 2){
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
            pushToTablesList(&globalHead,&globalTail,newTable);

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
              //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
              processesList_t *newTable = malloc(sizeof(processesList_t));
              newTable -> table = malloc(sizeof(processTable_t));
              copyTable(newTable -> table,currentProcessTable);
              pushToTablesList(&processesListHead,&processesListTail,newTable);
              pushToTablesList(&globalHead,&globalTail,newTable);
              //---------------------------- END --------------------------------
              if(currentProcessTable -> pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                //------------------------------- & ----------------------------
                //----- CHIUSURA LATO WRITE NEL CASO SIA DEFINITA UNA PIPE DI INPUT
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                if(outLogFile || uniLogFile){setenv_w(EV_STDOUTFILE,currentProcessTable -> tmpOutFile);}
                if(errLogFile  || uniLogFile){setenv_w(EV_STDERRFILE,currentProcessTable -> tmpErrFile);}
                if(!writeProcInfo){setenv_w(EV_PINFO_OUTFILE,currentProcessTable -> tmpProcInfoFile);}
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);

                if(currentProcessTable -> inputPipe != 1){//Se è definita una pipe di input per il  processo
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                while(processesListTail != NULL){
                  if(processesListTail -> table -> inputPipe != 1){
                    close(processesListTail-> table -> inputPipe);
                  }
                  if(processesListTail-> table -> outputPipe != 2){
                    close(processesListTail-> table -> outputPipe);
                  }
                  processesListTail = processesListTail -> next;
                }
                //---------------------------- END -----------------------------
                execvp(LOGGER_EXEC_PATH,exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable -> pid > 0){
                WAITING = TRUE;
                printf("wait = %d\n",WAITING );
                W8PID = currentProcessTable -> pid;
                printf("w8pid = %d\n",W8PID );
                while(WAITING){
                  sleep(1);
                }
                printf("W8STATUS = %d\n",W8STATUS );
                if( W8STATUS != EXIT_SUCCESS){
                  printf("skipping\n");
                  nextProcessTable -> skip = TRUE;
                }
                W8STATUS = -1;
                // int wstatus;
                // if(waitpid(currentProcessTable -> pid,&wstatus,WUNTRACED | WCONTINUED) > 0){
                //   if(WIFEXITED(wstatus)){
                //     int status = WEXITSTATUS(wstatus);
                //     if(status != EXIT_SUCCESS){
                //       nextProcessTable->skip = TRUE;
                //     }
                //     currentProcessTable -> status = status;
                //   }
                // }
              }
              else{
                exit_w(ERR_FORK_FAIL);
              }
            }
            else if(currentProcessTable -> skip == TRUE){
              nextProcessTable -> skip = currentProcessTable -> skip && FALSE;
            }
            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);
            }
            break;
          case OR:{


            //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
            // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
            //              e l'operando a SX di quest'ultimo ha fallito
            if(currentProcessTable -> skip == FALSE){

              //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
              currentProcessTable -> pid = fork();
              //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
              processesList_t *newTable = malloc(sizeof(processesList_t));
              newTable -> table = malloc(sizeof(processTable_t));
              copyTable(newTable -> table,currentProcessTable);
              pushToTablesList(&processesListHead,&processesListTail,newTable);
              pushToTablesList(&globalHead,&globalTail,newTable);
              //---------------------------- END --------------------------------
              if(currentProcessTable -> pid == 0){

                //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                //------------------------------- & ----------------------------
                //----- CHIUSURA LATO WRITE NEL CASO SIA DEFINITA UNA PIPE DI INPUT
                char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                if(outLogFile || uniLogFile){setenv_w(EV_STDOUTFILE,currentProcessTable -> tmpOutFile);}
                if(errLogFile  || uniLogFile){setenv_w(EV_STDERRFILE,currentProcessTable -> tmpErrFile);}
                if(!writeProcInfo){setenv_w(EV_PINFO_OUTFILE,currentProcessTable -> tmpProcInfoFile);}
                setenv_wi(EV_PIPE_IN,currentProcessTable->inputPipe);
                setenv_wi(EV_PIPE_OUT,currentProcessTable->outputPipe);

                if(currentProcessTable -> inputPipe != 1){//Se è definita una pipe di input per il  processo
                  close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                }
                while(processesListTail != NULL){
                  if(processesListTail -> table -> inputPipe != 1){
                    close(processesListTail-> table -> inputPipe);
                  }
                  if(processesListTail-> table -> outputPipe != 2){
                    close(processesListTail-> table -> outputPipe);
                  }
                  processesListTail = processesListTail -> next;
                }
                //---------------------------- END -----------------------------

                execvp(LOGGER_EXEC_PATH,exec_argv);
                exit_w(ERR_EXEC_FAIL);
              }
              else if(currentProcessTable -> pid > 0){
                WAITING = TRUE;
                printf("wait = %d\n",WAITING );
                W8PID = currentProcessTable -> pid;
                printf("w8pid = %d\n",W8PID );
                while(WAITING){
                  sleep(1);
                }
                printf("w8status = %d\n",W8STATUS );
                if( W8STATUS == EXIT_SUCCESS){
                  printf("skipping\n");
                  nextProcessTable -> skip = TRUE;
                }
                W8STATUS = -1;
                // int wstatus;
                // if(waitpid(currentProcessTable -> pid,&wstatus,0) > 0){
                //   printf("in if\n" );
                //   if(WIFEXITED(wstatus)){
                //
                //     int status = WEXITSTATUS(wstatus);
                //     printf("status = %d\n",status);
                //     if(status == EXIT_SUCCESS){
                //       printf("skippin next\n" );
                //       nextProcessTable -> skip = TRUE;
                //     }
                //     currentProcessTable -> status = status;
                //   }
                // }
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

            copyTable(currentProcessTable,nextProcessTable);
            clearTable(nextProcessTable);

            }
            break;

          break;

        }
        break;
      //---------------------------- CASE OPERATOR END -------------------------------



      //------------------------------- CASE REDIR_FILE -----------------------------------
      case REDIR_FILE :
        break;
      //------------------------------- CASE REDIR_FILE -----------------------------------



    }
    //----------------------- CONTROLLO SUL TIPO DI TOKEN ----------------------
    //----------------------------------- END ----------------------------------


    currentPointer += 1;
    if(currentPointer == nTokens){
      //-------------------- CONTROLLO SE IL COMANDO VA ESEGUITO -------------------------
                  // Ad esempio se in precedenza ho incontrato un operatore booleano (e.g. && )
                  //              e l'operando a SX di quest'ultimo ha fallito
                  if(currentProcessTable -> skip == FALSE){
                    printf("executing pwd\n");
                    //-------- FORK DI UN FIGLIO PER L'ESECUZIONE DEL LOGGER ---------
                    currentProcessTable -> pid = fork();
                    if(currentProcessTable -> pid == 0){

                      //-------- PREPARAZIONE AMBIENTE ED ARGOMENTI PER L'EXEC -------
                      //------------------------------- & ----------------------------
                      //----- CHIUSURA LATO WRITE NEL CASO SIA DEFINITA UNA PIPE DI INPUT
                      char ** exec_argv = getExecArguments(LOGGER_EXEC_NAME,currentProcessTable);
                      if(outLogFile || uniLogFile){setenv_w(EV_STDOUTFILE,currentProcessTable -> tmpOutFile);}
                      if(errLogFile  || uniLogFile){setenv_w(EV_STDERRFILE,currentProcessTable -> tmpErrFile);}
                      if(!writeProcInfo){setenv_w(EV_PINFO_OUTFILE,currentProcessTable -> tmpProcInfoFile);}
                      setenv_wi(EV_PIPE_IN,currentProcessTable -> inputPipe);
                      setenv_wi(EV_PIPE_OUT,currentProcessTable -> outputPipe);

                      if(currentProcessTable -> inputPipe != 1 && strcmp(currentProcessTable -> inputFile,"") == 0){//Se è definita una pipe di input per il  processo e NON è definito un file di input
                        //NB il controllo sulla presenza di un input file serve per garantire che inputPipe + 1 sia il lato WRITE di una pipe
                        close(currentProcessTable -> inputPipe +1); //vado a chiudere anche il lato write di quella pipe
                      }
                      while(processesListTail){
                        if(processesListTail -> table -> inputPipe != 1){
                          printf("closing in pipe\n");
                          close(processesListTail-> table -> inputPipe);
                        }
                        if(processesListTail-> table -> outputPipe != 2){
                          printf("closing out pipe\n");
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
                  printf("pushing to list 2\n" );
                  //---------------------------- END ---------------------------------
                  //----------- INSERIMENTO IN LISTA DELLA TAVOLA DEL PROCESSO -------
                  processesList_t *newTable = malloc(sizeof(processesList_t));
                  newTable->table = malloc(sizeof(processTable_t));
                  copyTable(newTable->table,currentProcessTable);
                  pushToTablesList(&processesListHead,&processesListTail,newTable);
                  pushToTablesList(&globalHead,&globalTail,newTable);
                  copyTable(currentProcessTable,nextProcessTable);
                  clearTable(nextProcessTable);
      //---------------------------- END --------------------------------
    }


  }
  //------------------- ALGORITMO DI GENERAZIONE DEI LOGGER --------------------
  //-------------------------------------- END ---------------------------------

    printTablesList(processesListHead,processesListTail);
  //---------------------------------- ATTESA DEI FIGLI ------------------------
  while ( ( wpid = wait(&childStatus) ) > 0){

      //Lista temporanea per la ricerca della tabella del figlio che ha terminato
      while(processesListTail != NULL){ //Scorro la lista finche non trovo una tabella con pid uguale al pid del processo appena terminato

        if(processesListTail -> table -> pid == wpid){
          //Se sono definite pipe le chiudo
          if( processesListTail -> table -> inputPipe != 1){
            close_w(processesListTail -> table -> inputPipe);
          }
          if(processesListTail -> table -> outputPipe != 2){
            close_w(processesListTail -> table -> outputPipe);
          }
          //Salvo lo stato del processo in tabella
          processesListTail -> table -> status = WEXITSTATUS(childStatus);
        }
        processesListTail = processesListTail -> next;
      }
      rewindLinkedList(&processesListHead,&processesListTail);
  }
  //--------------------------------- ATTESA DEI FIGLI -------------------------
  //-------------------------------------- END ---------------------------------

  printf("opening logs\n" );

  //------------------------------- APERTURA FILE DI LOG -----------------------
  //--------------------- E RIPOSIZIONAMENTO CURSORE -----------------------
  if(outLogFile){
    outLogFD = open_w(outLogFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
    lseek_w(outLogFD,0,SEEK_END);
   }
  if(errLogFile ){
     errLogFD = open_w(errLogFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP);
     lseek_w(errLogFD,0,SEEK_END);
    }

  if(uniLogFile){
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

  //  Il controllo xxxLogFile serve per determinare la scelta dell'utente in merito ai file di log
  //  writeProcInfo controlla se vanno loggate informazioni riguardo al processo
  //  eseguito. Uguale a NULL allora le informazioni vanno scritte

  //------------------------------- FORMATTING ---------------------------------
  if(outLogFile){
      byteWritten = write_w(outLogFD,FRM_EXP_SEPARATOR,strlen(FRM_EXP_SEPARATOR));
      byteWritten = write_w(outLogFD,FRM_INPUT_STRING,strlen(FRM_INPUT_STRING));
      byteWritten = write_w(outLogFD,rawInput,strlen(rawInput));
      byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));

  }
  if(errLogFile ){
    byteWritten = write_w(errLogFD,FRM_EXP_SEPARATOR,strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(errLogFD,FRM_INPUT_STRING,strlen(FRM_INPUT_STRING));
    byteWritten = write_w(errLogFD,rawInput,strlen(rawInput));
    byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));

  }
  if(uniLogFile){
    byteWritten = write_w(uniLogFD,FRM_EXP_SEPARATOR,strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(uniLogFD,FRM_INPUT_STRING,strlen(FRM_INPUT_STRING));
    byteWritten = write_w(uniLogFD,rawInput,strlen(rawInput));
    byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));

  }
  //------------------------------------ END -----------------------------------


  //----------------- LETTURA  INFORMAZIONI DEL PROCESSO -----------------------
  //  scorro la lista dei processi. Per ogni uno :
  //  1)  Se sono stati definiti i file temporanei, in caso vengono aperti
  //  2)  Aggiunge elementi di formattazione e informazioni riguardo il processo (comando,opzioni,...)
  //  3)  Se sono definiti legge il file process information (procInfo), output o error file
  //  4)  Infine rimuve i file temporanei e chiude i file descriptor
  while(processesListTail != NULL){

    if(outLogFile || uniLogFile){
      tmpOutFD = open_w(processesListTail -> table -> tmpOutFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP) ;
    }
    if(errLogFile  || uniLogFile){
      tmpErrFD = open_w(processesListTail -> table -> tmpErrFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP) ;
    }
    if(!writeProcInfo){
      tmpProcInfoFD = open_w(processesListTail -> table -> tmpProcInfoFile,O_RDWR|O_CREAT,S_IRWXU|S_IRGRP) ;
    }


    totWrittenOut = 0;
    totWrittenErr = 0;
    totWrittenUni = 0;


    //---------------- SCRITTURA INFORMAZIONI DEL COMANDO ESEGUITO -----------------
    //Scrittura del nome del comando e delle opzioni utilizzate nell'invocazione
    //Controllando sempre che file sono stati definiti dall'utente per la scrittura
    if(outLogFile){
      //Se è la prima opzione scrivo anche il nome del comando
        byteWritten = write_w(outLogFD,FRM_COMMAND,strlen(FRM_COMMAND));
        byteWritten = write_w(outLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
        byteWritten = write_w(outLogFD,"\n",strlen("\n"));
        byteWritten = write_w(outLogFD,FRM_OPTIONS,strlen(FRM_OPTIONS));

    }
    if(errLogFile ){
      //Se è la prima opzione scrivo anche il nome del comando
      byteWritten = write_w(errLogFD,FRM_COMMAND,strlen(FRM_COMMAND));
      byteWritten = write_w(errLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
      byteWritten = write_w(errLogFD,"\n",strlen("\n"));
      byteWritten = write_w(errLogFD,FRM_OPTIONS,strlen(FRM_OPTIONS));

    }
    if(uniLogFile){
      //Se è la prima opzione scrivo anche il nome del comando
      byteWritten = write_w(uniLogFD,FRM_COMMAND,strlen(FRM_COMMAND));
      byteWritten = write_w(uniLogFD,processesListTail -> table -> command, strlen(processesListTail -> table -> command));
      byteWritten = write_w(uniLogFD,"\n",strlen("\n"));
      byteWritten = write_w(uniLogFD,FRM_OPTIONS,strlen(FRM_OPTIONS));

    }

    for(i=0;i<processesListTail -> table -> nOptions;i++){
      //Controllo File di output
      if(outLogFile){
        byteWritten = write_w(outLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
        byteWritten = write_w(outLogFD," ",strlen(" "));

      }

      //Controllo File di errore
      if(errLogFile ){
        byteWritten = write_w(errLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
        byteWritten = write_w(errLogFD," ",strlen(" "));
      }

      //Controllo File di output ed errore unificati
      if(uniLogFile){
        byteWritten = write_w(uniLogFD,processesListTail -> table -> options[i], strlen(processesListTail -> table -> options[i]));
        byteWritten = write_w(uniLogFD," ",strlen(" "));
      }
    }

    if(outLogFile){
      byteWritten = write_w(outLogFD, "\n", strlen("\n"));
      byteWritten = write_w(outLogFD,FRM_FLD_SEPARATOR, strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(outLogFD,FRM_PROC_INFO, strlen(FRM_PROC_INFO));
      byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));
    }

    if(errLogFile ){
      byteWritten = write_w(errLogFD, "\n", strlen("\n"));
      byteWritten = write_w(errLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(errLogFD,FRM_PROC_INFO, strlen(FRM_PROC_INFO));
      byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));
    }

    if(uniLogFile){
      byteWritten = write_w(uniLogFD, "\n", strlen("\n"));
      byteWritten = write_w(uniLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(uniLogFD,FRM_PROC_INFO, strlen(FRM_PROC_INFO));
      byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
    }
    //-------------------------------------- END -------------------------------------



    //------------------ LETTURA FILE TMP CONTENENTE PROC. INFO ----------------------
        if(!writeProcInfo){
          //Riposiziono l'indice di lettura per il file temporaneo
          lseek_w(tmpProcInfoFD,0,SEEK_SET);
          while( (byteRead = read_w(tmpProcInfoFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
            //Controllo dove scrivere le informazioni e le scrivo
            if(outLogFile){byteWritten = write_w(outLogFD,readBuffer,byteRead);}
            if(errLogFile ){byteWritten = write_w(errLogFD,readBuffer,byteRead);}
            if(uniLogFile){byteWritten = write_w(uniLogFD,readBuffer,byteRead);}
          }
          memset(readBuffer,0,CMD_OUT_BUFF_SIZE);//Pulizia del buffer
        }


    //-------------------------------------- END -----------------------------------


    if(outLogFile){
      byteWritten = write_w(outLogFD,FRM_FLD_SEPARATOR, strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(outLogFD,FRM_PROC_OUT, strlen(FRM_PROC_OUT));
      byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));
    }

    if(errLogFile ){
      byteWritten = write_w(errLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(errLogFD,FRM_PROC_ERR, strlen(FRM_PROC_ERR));
      byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));
    }

    if(uniLogFile){
      byteWritten = write_w(uniLogFD,FRM_FLD_SEPARATOR,strlen(FRM_FLD_SEPARATOR));
      byteWritten = write_w(uniLogFD,FRM_PROC_OUT, strlen(FRM_PROC_OUT));
      byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
    }


    //---------------------- LETTURA FILE TMP ASSOCIATO A STD. OUT -----------------
      if(outLogFile || uniLogFile){
        lseek_w(tmpOutFD,0,SEEK_SET);//Riposiziono l'indice di lettura per il file temporaneo
        while( (byteRead = read_w(tmpOutFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
        //Controllo dove scrivere le informazioni e le scrivo
        if(outLogFile){
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
        if(uniLogFile){
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
        memset(readBuffer,0,CMD_OUT_BUFF_SIZE);//Pulizia del buffer
      }
    //-------------------------------------- END -----------------------------------


    if(outLogFile){
      byteWritten = write_w(outLogFD,"\n\n",strlen("\n\n"));
    }

    if(uniLogFile){
      byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
      byteWritten = write_w(uniLogFD,FRM_PROC_ERR,strlen(FRM_PROC_ERR));
      byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
    }


    //------------------- LETTURA FILE TMP ASSOCIATO A STD. ERR --------------------
      if(errLogFile  || uniLogFile){
        //Riposiziono l'indice di lettura per il file temporaneo
        lseek_w(tmpErrFD,0,SEEK_SET);
      while( (byteRead = read_w(tmpErrFD,readBuffer,CMD_OUT_BUFF_SIZE)) > 0){
        //Controllo dove scrivere le informazioni e le scrivo
        if(errLogFile ){

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
        if(uniLogFile){

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
      memset(readBuffer,0,CMD_OUT_BUFF_SIZE);//Pulizia del buffer
      }
    //---------------------------------- END ---------------------------------------


    // ---------------------------- FORMATTAZIONE ------------------------------
    if(outLogFile){
      //manca per un motivo :)
      byteWritten = write_w(outLogFD,FRM_CMD_SEPARATOR, strlen(FRM_CMD_SEPARATOR));
      byteWritten = write_w(outLogFD,"\n",strlen("\n"));
    }

    if(errLogFile ){
      byteWritten = write_w(errLogFD,"\n\n",strlen("\n\n"));
      byteWritten = write_w(errLogFD,FRM_CMD_SEPARATOR, strlen(FRM_CMD_SEPARATOR));
      byteWritten = write_w(errLogFD,"\n",strlen("\n"));
    }

    if(uniLogFile){
      byteWritten = write_w(uniLogFD,"\n\n",strlen("\n\n"));
      byteWritten = write_w(uniLogFD,FRM_CMD_SEPARATOR, strlen(FRM_CMD_SEPARATOR));
      byteWritten = write_w(uniLogFD,"\n",strlen("\n"));
    }
    //----------------------------- END ----------------------------------------


    //----------------- PULIZIA E RIMOZIONE DEI FILE TEMPORANEI ----------------
    //------------ CHIUSURA DEI FILE DESCRIPTORS ASSOCIATI AI FILE TMP ---------
    if(outLogFile || uniLogFile){

      //Eliminazione del associazione nel file system
      unlink(processesListTail -> table -> tmpOutFile);
      //Chiusura
      close(tmpOutFD);//Chiusura del file descriptor associato al file temporaneo proc. Info

    }
    if(errLogFile  || uniLogFile){
      unlink(processesListTail -> table -> tmpErrFile);//Eliminazione del file temporaneo di errore
      close(tmpErrFD);//Chiusura del file descriptor associato al file temporaneo di output

    }
    if(!writeProcInfo){
      unlink(processesListTail -> table -> tmpProcInfoFile);//Eliminazione del file temporaneo di proces Info
      close(tmpProcInfoFD);//Chiusura del file descriptor associato al file temporaneo di err

    }
    //---------------------------------- END -----------------------------------
    //---------------------------------- END -----------------------------------




    processesListTail = processesListTail -> next;//Lettura del prossimo elemento in lista
  }
  //-------------------------------------- END -------------------------------------------



  //------------------- FORMATTAZIONE PER LA PROSSIMA ESPRESSIONE -------------
  if(outLogFile){
    byteWritten = write_w(outLogFD,FRM_EXP_SEPARATOR, strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(outLogFD,"\n",strlen("\n"));
  }

  if(errLogFile ){
    byteWritten = write_w(errLogFD,FRM_EXP_SEPARATOR, strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(errLogFD,"\n",strlen("\n"));
  }

  if(uniLogFile){
    byteWritten = write_w(uniLogFD,FRM_EXP_SEPARATOR, strlen(FRM_EXP_SEPARATOR));
    byteWritten = write_w(uniLogFD,"\n",strlen("\n"));
  }
  //------------------------------------ END -----------------------------------



  //------------------------ CHIUSURA FILES DI LOG -----------------------------
  if(outLogFile){close(outLogFD);}
  if(errLogFile){close(errLogFD);}
  if(uniLogFile){close(uniLogFD);}
  //------------------------------------ END -----------------------------------



  //--------------------------- REWIND DELLA LISTA -----------------------------
  rewindLinkedList(&processesListHead,&processesListTail);
  //------------------------------------ END -----------------------------------



//------------------------------------ END -------------------------------------
//------------------------------------ && --------------------------------------
//------------------------------------ END -------------------------------------






  //-------------------------------- USCITA ------------------------------------
                              exit(EXIT_SUCCESS);
}
