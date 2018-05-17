#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//Boolean MACROS
#define TRUE 1
#define FALSE 0

//Pipes MACROS
#define READ 0
#define WRITE 1

//processIOtable parameters' MACROS
#define MAX_CMD_LEN 50
#define MAX_ARGUMENTS 10
#define MAX_ARG_LEN 10
#define MAX_IN_FILE_LEN 15
#define MAX_OUT_FILE_LEN 15

typedef int bool;

struct Token  {
  char type[10];
  char value[50];
};

struct processIOtable {
  char command [MAX_CMD_LEN];
  char options [MAX_ARGUMENTS][MAX_ARG_LEN];
  int inputPipe;
  int outputPipe;
  char inputFile[MAX_IN_FILE_LEN];
  char outputFile[MAX_OUT_FILE_LEN];
};

struct pidsList{
  pid_t pid;
  struct pidsList* next;
  struct pidsList* prev;
};

struct pipesList{
  int pipe[2];
  struct pipesList* next;
  struct pipesList* prev;
};


//Controller si aspetta come PRIMO argomento una stringa SENZA ;
int main(int argc,char** argv){
  //Esempio di invocazione al tokenizer
  //tokenList = tokenize(argv[1],&len,&nCommands); To Do

  //Example INPUT
  struct Token token_0;
    strcpy(token_0.type,"command");
    strcpy(token_0.value,"ls");
  struct Token token_1;
    strcpy(token_1.type,"operator");
    strcpy(token_1.value,"|");
  struct Token token_2;
    strcpy(token_2.type,"command");
    strcpy(token_2.value,"wc");
  struct Token token_3;
    strcpy(token_3.type,"operator");
    strcpy(token_3.value,"|");
  struct Token token_4;
    strcpy(token_4.type,"command");
    strcpy(token_4.value,"pwd");
  struct Token token_5;
    strcpy(token_5.type,"operator");
    strcpy(token_5.value,"&&");
  struct Token token_6;
    strcpy(token_6.type,"command");
    strcpy(token_6.value,"echo 1");
  struct Token inputTokens[] = {token_0,token_1,token_2,token_3,token_4,token_5,token_6};
  int inputLength = 7;
  int nCommands = 4;

  //Algorithm's Data Structures

  //Token in analisi
  struct Token* currentToken;

  //Tabella contentente informazioni sul processo in analisi
  struct processIOtable currentProcessTable;
  //Tabella contentente informazioni sul prossimo processo in analisi
  struct processIOtable nextProcessTable;
  //Lista delle tabelle dei processi generati
  struct processIOtable processesTables [nCommands];

  //Lista dei processi generati
  //Puntatori al primo e all' ultimo processo in lista
  struct pidsList* pidsHead,pidsTail;
  //pidsTail = pidsHead;

  //Lista delle pipes generate
  //Puntatori alla prima e all' ultima pipe in lista
  struct pipesList* pipesHead,pipesTail;
  //pipesTail = pipesHead;

  //Puntatore all'indice del token in analisi nell'input
  int currentPointer = 0;
  int optionsCounter = 0;

  //Flag di errore per la terminazione dell'algoritmo
  bool error = FALSE;

  while( (currentPointer < inputLength) && (!error) ){ //Condizioni di uscita dall'algoritmo : ho finito la scansione (i.e. currentPointer == inputLength) oppure sono in errore
    //Leggo un token
    currentToken = &inputTokens[currentPointer];
    printf("currentToken = [ %s , %s ]\n",currentToken->type,currentToken->value);

    //Controllo sul tipo di token
    if( strcmp(currentToken->type,"command") == 0 ){ //Il token è un COMANDO
      //Inserisco in tabella il comando
      strcpy(currentProcessTable.command,currentToken->value);
      //Azzero il contatore delle opzioni dato che ho un nuovo COMANDO
      optionsCounter = 0;
      //Avanzo il puntatore al prossimo Token
      currentPointer += 1;
    }
    else if( strcmp(currentToken->type,"option") == 0){ //Il token è un OPZIONE
      //Controllo sul numero di argomenti per garantire che sia minore di MAX_ARGUMENTS
      if( optionsCounter > MAX_ARGUMENTS){ //Ho più opzioni di quelle inseribili -> errore (gestirlo dinamicamente magari?)
        error = TRUE;
      }
      else{
        //Inserisco l'opzione nella currentProcessTable in options[optionsCounter]
        strcpy(currentProcessTable.options[optionsCounter],currentToken->value);
        //Aumento il contatore delle opzioni
        optionsCounter += 1;
        //Avanzo il puntatore al prossimo token
        currentPointer += 1;
      }
    }
    else if( strcmp(currentToken->type,"operator") == 0 ){ //Il token è un OPERATORE
      //Controllo sul tipo di OPERATORE
      if( strcmp(currentToken->value,"|") == 0 ){ //L'OPERATORE è una PIPE

        //Allocazione di una nuova PIPE
        struct pipesList* newPIPE = malloc(sizeof(struct pipesList));
        //Creazione della pipe
        pipe(newPIPE->pipe);
        //Controllo se la lista è vuota
        //AFFERMATIVO : faccio puntare head e tail alla pipe appena creata
        if( *pipesHead && *pipesTail ){
          pipesHead = newPIPE;
          pipesTail = newPIPE;
        }
        //NEGATIVO : aggiungo la nuova pipesList alla lista di pipes
        else{
          newPIPE->prev = pipesHead;
          pipesHead->next = newPIPE;
          pipesHead = newPIPE;
        }

        //Aggiorno tabella dei processi
        currentProcessTable.outputPipe = newPIPE[WRITE];
        nextProcessTable.inputPipe = newPIPE[READ];

        //Preparazione degli argomenti da passare alla exec del figlio che verrà generato al prossimo punto
          //Help needed!!
          //Essendo in numero variabile va usata execv magari anche execvp se vogliamo andare sull sentiero di PATH etc...(no pun intended!)
        //

        //Creazione figlio per l'esecuzione di currentProcessTable
        struct pidsList* newCHILD = malloc(sizeof(pidsList));
        //Controllo se la lista è vuota :
        //AFFERMATIVO : faccio puntare head e tail al processesList appena creato
        if( pidsHead && pidsTail ){
          pidsHead = newCHILD;
          pidsTail = newCHILD;
        }
        //NEGATIVO : aggiungo newCHILD alla coda di pids
        else{
          newCHILD->prev = pidsHead;
          pidsHead->next = newCHILD;
          pidsHead = newCHILD;
        }
        newCHILD->pid = fork();
        //Codice del figlio
        if( newCHILD->pid == 0 ){
          printf("Greetings from child :)\n");
          //Qua andrà effettuata la exec sull'immagine del processo che gestisce le redirezioni, loggin' parziale, etc..
          exit(0);
        }
        //Codice del padre (aka il controller)
        else if( newCHILD->pid > 0){
          //Swap della tabella currentProcessTable con nextProcessTable e creazione/pulizzia della nuova nextProcessTable
          //NB : per questa stesura sto salvando tutte le tabelle in una lista delle tabelle per controllarle alla fine e verificare che tutto sia come ce lo aspettiamo, probabilmente per la versione finale è sufficiente "pulire" la nextProcessTable
          strcpy(currentProcessTable.command,nextProcessTable.command);
          int i;
          for(i=0;i<MAX_ARGUMENTS;i++){
            //Ci metto una stringa vuota cosi che se nextProcessTable.options < currentProcessTable.options quello che sarà poi il currentProcessTable non si ritrova per sbaglio argomenti di qualche comando passato
            strcpy(currentProcessTable.options[i],"");
            strcpy(currentProcessTable.options[i],nextProcessTable.options[i]);
          }
          currentProcessTable.inputPipe = nextProcessTable.inputPipe;
          currentProcessTable.outputPipe = -1;
          strcpy(currentProcessTable.inputFile,"");
          strcpy(currentProcessTable.outputFile,"");

          //Avanzo il puntatore al prossimo token
          currentPointer += 1;
        }
        //Errore nella creazione del processo figlio
        else{
          printf("Errore nella creazione del figlio per il comando ' %s '\n",currentProcessTable->command);
          error = TRUE;
        }

      }
      else if( strcmp(currentToken->value,"&&") == 0){ //L'OPERATORE è un AND

      }
      else if( strcmp(currentToken->value,"||") == 0){ //L'OPERATORE è un OR

      }
      else if( strcmp(currentToken->value,">") == 0){ //L'OPERATORE è un OUTPUT REDIRECT

      }
      else if( strcmp(currentToken->value,"<") == 0){ //L'OPERATORE è un INPUT REDIRECT

      }
      else{ //L'OPERATORE NON è stato riconosciuto come un operatore valido -> errore
        printf("Errore : Token = [ %s , %s ] NON COMBACIA CON ALCUN OPERATORE\n",currentToken->type,currentToken->value);
        error = TRUE;
      }
    }
    else{ //Il token NON è stato riconosciuto -> errore
      printf("Errore : Token = [ %s , %s ] NON RICONOSCIUTO\n",currentToken->type,currentToken->value);
      error = TRUE;
    }
  }



}
