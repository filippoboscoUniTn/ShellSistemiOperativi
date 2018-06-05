//------------------------- EXTERNAL LIBRARIES INCLUSION -----------------------
#include "libs/std_libraries.h"
#include "libs/macros.h"
#include "libs/types.h"
#include "libs/functions.h"
//-------------------------------------- END -----------------------------------

siginfo_t *child_1_info, *child_2_info;

void sigchld_handler_parent(int signum, siginfo_t *info, void *ucontext){

//  printf("Parent_Handler> SIGCHLD handler\n");
//  printf("Parent> signum = %d\n",signum);
//  printf("Parent> si_pid = %d\n",info -> si_pid);
//  printf("Parent> si_signo = %d\n",info -> si_signo);
  printf("Parent_Handler> si_status = %d\n",info -> si_status);
  child_1_info = info;

  return;
}

void sigchld_handler_child_1(int signum, siginfo_t *info, void *ucontext){

  //printf("Child_1_Handler> SIGCHLD handler\n");
  //printf("Child_1> signum = %d\n",signum);
  //printf("Child_1> si_pid = %d\n",info -> si_pid);
  //printf("Child_1> si_signo = %d\n",info -> si_signo);
  printf("Child_1_Handler> si_status = %d\n",info -> si_status);
  child_2_info = info;
  return;
}




int main(int argc,char **argv){
  printf("hello from parent\n");

  int children [3];
  int childStatus;
  int wpid;
  struct sigaction sigact;
  sigact.sa_flags = SA_SIGINFO;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_sigaction = sigchld_handler_parent;
  sigaction(SIGCHLD,&sigact,NULL);


  children[0] = fork();
  if(children[0] == 0){ //CHILD_1

    printf("hello from child_1\n");
    int child_2;

    struct sigaction sigact_1;
    sigact_1.sa_flags = SA_SIGINFO;
    sigemptyset(&sigact_1.sa_mask);
    sigact_1.sa_sigaction = sigchld_handler_child_1;
    sigaction(SIGCHLD,&sigact_1,NULL);

    child_2 = fork();
    if(child_2 == 0){ //CHILD_2

      printf("hello from child_2\n");
      exit(EXIT_FAILURE);

    }
    else if(child_2 < 0){
      perror("error forking\n");
    }

    else{ //CHILD_1
      int child_2Status;
      while ( ( wpid = wait(&childStatus) ) > 0){
        if(WIFEXITED(childStatus)){
          child_2Status = WEXITSTATUS(childStatus);
          printf("Child_1> WEXITSTATUS(childStatus) = %d\n",child_2Status );
        }
      }
      exit(child_2Status);
      //exit(child_2Status);
    }

  }

  else if(children[0] < 0){
    perror("error forking\n");
  }

  else{ //PARENT

    while ( ( wpid = wait(&childStatus) ) > 0){
      if(WIFEXITED(childStatus)){
        printf("Parent> child_1 status = %d\n",WEXITSTATUS(childStatus) );
      }
      printf("Parent> childStatus = %d\n",childStatus );
    }

  }











  exit(EXIT_SUCCESS);
}
