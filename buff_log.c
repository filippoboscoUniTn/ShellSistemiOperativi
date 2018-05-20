#include "std_libraries.h"

#define READ 0
#define WRITE 1

int main(int argc, char **argv){

	printf("buff_log started.\n");

	char *frasi[7];
	frasi[0] = "ciao prima riga\n";
	frasi[1] = "ciao seconda riga\n";
	frasi[2] = "ciao terza riga\n";
	frasi[3] = "ciao quarta riga\n";
	frasi[4] = "ciao quinta riga\n";
	frasi[5] = "ciao sesta riga\n";
	frasi[6] = "ciao settima riga\n";

	int i;
	for(i = 0; i < 7; i++){
		printf("frasi[%i] --> %s\n", i, frasi[i]);
	}

	int pipes[2];
	pipe(pipes);

	int pid = fork();

	//child, writer
	if(pid == 0){
		printf("[CHILD] i will write to pipe (%i)\n", pipes[WRITE]);
		close(pipes[READ]);

		int j;
		ssize_t wroteb;
		for(j = 0; j < 7; j++){
			wroteb = write(pipes[WRITE], frasi[j], strlen(frasi[j]));
			printf("[CHILD] wrote string: %s   |  bytes: %zi\n", frasi[j], wroteb);
		}
		printf("[CHILD] finished writing to pipe");
	}
	//father, reader and logger
	else if(pid > 0){
		close(pipes[WRITE]);
		char *filename = "/Users/nik/Desktop/out.txt";
		int outf;
		outf = open(filename, O_WRONLY | O_CREAT, 0755);
		if(outf == -1){
			printf("error opening output file\n");
			exit(EXIT_FAILURE);
		}

		printf("[FATHER] i will read from pipe (%i) and write to %s (%i)\n", pipes[READ], filename, outf);

		char *buffer = malloc(sizeof(char) * 32);
		
		if(buffer == NULL){
			printf("malloc error\n");
			exit(EXIT_FAILURE);
		}

		int c;
		ssize_t p_readb, a_readb;
		p_readb = 0;
		for(c = 0; c < 10; c++){
			a_readb = read(pipes[READ], buffer, 32);

			if(a_readb){
				write(outf, buffer, a_readb);
				printf("[FATHER] read string: %s   |  bytes: %zi\n", buffer, a_readb );
			}
		}



		wait(NULL);
		printf("[FATHER] My child has finished\nBye bye.\n");
	}
	else{
		printf("forking failed\n");
		exit(EXIT_FAILURE);
	}






















	exit(EXIT_SUCCESS);
}
