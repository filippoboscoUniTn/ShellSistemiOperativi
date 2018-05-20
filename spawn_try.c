#include "std_libraries.h"

int main(int argc, char **argv){

	printf("Spawner started.\n");

	//allocating argc elements of type char*
	//argc - 1 for holding all the parameters + 1 for NULL pointer
	char *args[argc];
	char *cmd;

	int i;
	for(i = 1; i < argc; i++){
		args[i - 1] = malloc( (sizeof(char) * strlen(argv[i])) + 1 );
		strcpy(args[i - 1], argv[i]);
	}
	args[argc - 1] = NULL;

	cmd = malloc( (sizeof(char) * strlen(args[0])) + 1 );
	strcpy(cmd, args[0]);

	int pid = fork();

	//child
	if(pid == 0){
		printf("[CHILD] i will start\n");
		printf("cmd --> %s\n", cmd);
		int j;
		for(j = 0; args[j] != NULL; j++){
			printf("argv[j] --> %s\n", args[j]);
		}

		execvp(cmd, args);
		printf("exec failed.\n");
	}
	//father
	else if(pid > 0){
		printf("[FATHER] I wait...\n");
		wait(NULL);
		printf("[FATHER] My child has finished\n");
	}
	else{
		printf("forking failed\n");
		exit(EXIT_FAILURE);
	}






















	exit(EXIT_SUCCESS);
}
