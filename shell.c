
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <fcntl.h>


#define MAX_ARGS 128
#define ARGS_MAX_LEN 64

int main(int argc,char** argv){



  char args[MAX_ARGS][ARGS_MAX_LEN];
  char* taskControllerImg = "taskController";

  //Controllo argomenti
  if( argc <= 1 || ( (MAX_ARGS - argc +1) < 0 ) ){ //Errore nell'invocare il programma, 1 < numero argomenti < MAX_ARGS
    printf("Error : Incrorrect usage!\n");
    exit(EXIT_FAILURE);
  }
  //Copia dei comandi e degli operatori
  int arg;
  for(arg=0;arg <(argc-1);arg++){
    printf("arg = %d\n",arg);
    strcpy(args[arg],argv[arg+1]);
    printf("Shell's argument #%d = %s\n",arg,args[arg]);
  }
  strcpy(args[arg+1],"NULL");

  pid_t taskController = fork();
  if(taskController == 0){ //TaskController
    char* arguments[] = {"taskController","ls","|","wc",NULL};
    //Esecuzione taskController , usare execle || execvpe per passare un ambiente (i.e. per effettuare overide del PATH in cui cercare i comandi da eseguire!)
    if( execvp("taskController",arguments) == -1){
      perror("error exec");
      exit(EXIT_FAILURE);
    }
  }
  else if(taskController > 0){ //Main
    printf("In parent\n");
    //Main si metter in attesa che finisca il taskController
    wait(NULL);
  }
  else{ //Errore nel creare il taskController
    perror("Error executing task Controller\n");
    exit(EXIT_FAILURE);
  }


}
