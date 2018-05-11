#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#define READ 0
#define WRITE 1
#define BUFFSIZE 1024
#define TRUE 1

int main(int argc,char **argv){
	printf("before init\n");

	//questa è la parte che contiene i due programmi che dobbiamo lanciare
	char args[2][64];
	strcpy(args[0], "ls\0");
	strcpy(args[1], "grep\0");

	printf("after strcpy\n");

    //1) Set up delle pipe di IPC tra output arg1, input arg2
    int pipes [2][2]; //Fd associati alle pipe -> pipe[i][WRITE] -> input processo i-esimo, pipe[i][READ] -> output procecsso i-esimo
    int logFiles[2]; //Per ogni 'programma' un file di log
    int processCounter;
    char buffer[2][BUFFSIZE];
		char logFileName[64];

    for(processCounter = 0; processCounter < 2; processCounter++){

        pipe(pipes[processCounter]);
        strcpy(logFileName, "logFile_");
		strcat(logFileName, args[processCounter]); //attacca il nome del programma al nome del file di log
		strcat(logFileName, ".txt");
        logFiles[processCounter] = open(logFileName, O_RDWR|O_CREAT, 0755); //Creazione File di log

	}
		printf("after initial for\n");

        pid_t pid = fork();
        if(pid < 0){ //errore fork
            perror("error forking child\n");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){ //primo figlio, writer
			printf("first child\n");
			//Chiude pipe non necessarie
			close(pipes[0][READ]);
			close(pipes[1][READ]);
			close(pipes[1][WRITE]);

			//Redirigi output in pipe[0][WRITE]
			dup2(pipes[0][WRITE],STDOUT_FILENO);

			//per provare se dup2 funziona correttamente
			//stampa in stdout (e non anche a monitor)
			//la riga non contiene la lettera 'a' in modo da comparire nel log di ls, ma non nell'output di "grep a"
			fprintf(stdout, "this shouldn't be printed, dup2 STDOUT_FILENO\n");

			//Chiusura pipe[0][WRITE]
			close(pipes[0][WRITE]);
			//Exec
			execl("/bin/ls","ls", "-l", NULL);
			//ERROR
			perror("error executing ls");
			exit(EXIT_FAILURE);

        }
        else{ //Padre
			pid_t pid2 = fork();
			if(pid2 < 0){ //errore fork
				perror("error forking 2nd child");
	            exit(EXIT_FAILURE);
			}
			else if(pid2 == 0){//secondo figlio, reader
				printf("second child\n");
				//Chiude pipe non necessarie
				close(pipes[1][WRITE]);
				close(pipes[0][READ]);
				close(pipes[0][WRITE]);

				//Redirigi output in pipe[0][WRITE]
				dup2(pipes[1][READ],STDIN_FILENO);
				//Chiusura pipe[0][WRITE]
				close(pipes[1][READ]);
				//Exec
	            execl("/usr/bin/grep","grep", "a", NULL);
	            //ERROR
	            perror("error executing grep");
	            exit(EXIT_FAILURE);

			}
			else{ //Padre
				printf("father\n");

				//Chiude pipe non necessarie
				close(pipes[0][WRITE]);
				close(pipes[1][READ]);

				int processCounter;
				//readb = byte letti
				//writtenb = byte scritti
				ssize_t readb, writtenb;

				//serviva solo per far finire il while
				//dobbiamo fare degli handler di SIGPIPE qua nel padre per gestire quando finiscono i programmi
				int c;
				c = 0;

				while(c < 10){//continua a leggere, loggare e scrivere i buffer
					for(processCounter = 0; processCounter < 1; processCounter++){

						//prova a leggere byte dalla pipe del primo programma
						readb = read(pipes[0][READ], buffer[processCounter], BUFFSIZE);

						if(readb < 0){ //errore leggendo dalla pipe
							perror("error reading from pipe");
							exit(EXIT_FAILURE);
						}
						else if(readb > 0){ //se sono stati letti readb bytes
							//scrive nel log
							writtenb = write(logFiles[processCounter], buffer[processCounter], readb);
							//scrive nella pipe da cui il secondo programma legge STDIN
							writtenb = write(pipes[processCounter + 1][WRITE], buffer[processCounter], readb);
						}
					}
					c++;
				}
			}
        }
}
