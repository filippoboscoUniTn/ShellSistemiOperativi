/*
	CHANGES:
	- pensare a cosa deve essere singolarmente passato come parametro e cosa può essere passato come variabile d'ambiente
*/


/*
	this is the command logger source code
	this program will be called from the shell for every command to be executed
	the shell will pass the arg_info[] row associated with the command to execute
	the row will hold these arguments in the following order:
		1) command's name (as argv)
		2) command's parameters (if present) (as argv)
		3) temporary folder pathname in which to save partial logs (as env)
		4) shell's parameters' array for knowing what informations to log, and where to save it (as env)

	the logger will behave as following:
		1) catch the 1st arg treated as command's name
		2) catch the 2nd arg treated as command's parameters (if present)
		3) start scanning all environment variables for catching:
			- temporary folder pathname in which to save partial logs
			- shell's parameters' array for knowing what informations to log, and where to save it
			- pipes's file descriptors
		4) based on the informations to save, it will open output stream files, get process informations, link pipes and allocate buffers
		5) fork --> child will execute the command
				--> father will read input pipes, log to files and then write to output pipes
		6) when the child finishes, the father flushes the buffers, closes output streams, notify the controller and then exit
*/

#include "macros.h"
#include "std_libraries.h"
#include "types.h"

#define KEEP 0
#define OVERWRITE 1


//function that simply remove the argv[0]
//iterates copying from argv to args
//appends NULL to the end of args
void catch_args(int argc, char **argv, char **args){

	int i; //for index
	for(i = 1; i < argc; i++){

		//allocates memory in args
		args[i - 1] = malloc( (sizeof(char) * strlen(argv[i])) + 1 );

		//copies from argv to args
		strcpy(args[i - 1], argv[i]);
	}

	//lat element is NULL pointer for termination
	args[argc - 1] = NULL;

	return;
}

void link_pipe(int _source, int _destination){
	dup2(_destination, _source);
	close(_destination);
	return;
}


int main(int argc, char **argv){

	//if no arguments, error
	if(argc < 2){
		printf("logger error: no arguments!\n");
		exit(EXIT_FAILURE);
	}

	//allocating argc elements of type char*
	//argc - 1 for holding all the parameters + 1 for NULL pointer
	char *args[argc];
	char *cmd;

	catch_args(argc, argv, args); //removes the argv[0] for passing it to exec
	cmd = malloc( (strlen(args[0]) * sizeof(char)) + 1 ); //allocating space for command's name
	strcpy(cmd, args[0]); //catching command's name




	printf("cmd --> %s\n", cmd);
	int j;
	for(j = 0; args[j] != NULL; j++){
		printf("argv[%i] --> %s\n", j, args[j]);
	}

	printf("before strcpy\n");




	//these variables will be put in the environment by the controller
	char *t_out = malloc( 200 * sizeof(char) );
	strcpy(t_out, "/Users/nik/Desktop/stdout/");
	strcat(t_out, cmd);
	strcat(t_out, ".txt");
	char *t_err = malloc( 200 * sizeof(char) );
	strcpy(t_err, "/Users/nik/Desktop/stderr/");
	strcat(t_err, cmd);
	strcat(t_err, ".txt");

	printf("after strcpy\n");

	setenv(EV_MAXLEN, "200", OVERWRITE);
	setenv(EV_STDOUTFILE, t_out, OVERWRITE);
	setenv(EV_STDERRFILE, t_err, OVERWRITE);
	setenv(EV_PIPE_IN, "0", OVERWRITE);
	setenv(EV_PIPE_OUT, "1", OVERWRITE);

	char *out_pathname = NULL;
	char *err_pathname = NULL;
	char *proc_info_pathname = NULL;
	int pipe_in = -1;
	int pipe_out = -1;

	printf("before getenvs and atois\n");

	out_pathname = getenv(EV_STDOUTFILE);
	err_pathname = getenv(EV_STDERRFILE);
	proc_info_pathname = getenv(EV_PINFO_OUTFILE);
	pipe_in = atoi(getenv(EV_PIPE_IN));
	pipe_out = atoi(getenv(EV_PIPE_OUT));

	printf("after getenvs and atois\n");

	printf("outf_pathname --> %s\n", out_pathname);
	printf("errf_pathname --> %s\n", err_pathname);
	printf("proc_infof_pathname --> %s\n", proc_info_pathname);
	printf("pipe_in --> %i\n", pipe_in);
	printf("pipe_out --> %i\n", pipe_out);

	int outf = -1;
	int errf = -1;
	int proc_infof = -1;

	int pipes[2];
	pipes[0] = -1;
	pipes[1] = -1;
	char *buffer;

	//stdin has to be always linked to the input pipe
	//!!!!!!!!!!!!!! DEBUg has to be activated!
	//link_pipe(STDIN_FILENO, pipe_in);

	//if we don't have to log the stderr, we don't redirect anything
	//instead, if we have to log it, we redirect the stderr to the log's file descriptor
	if(err_pathname != NULL){
		errf = open(err_pathname, O_WRONLY | O_CREAT, 0755);
		printf("after opening the errfile errf --> %i\n", errf);
		link_pipe(STDERR_FILENO, errf);
	}

	//if we don't have to save the output we just redirect stdout to pipe_out
	if(out_pathname == NULL){
		printf("linked stdout to pipe_out\n");
		link_pipe(STDOUT_FILENO, pipe_out);
	}

	//if we have to log the stdout
	//we redirect the stdout to the stdout pipe
	//allocate the buffer
	//and open the file stream to the log file
	if(out_pathname != NULL){

		printf("before stdout buffer allocation\n");

		if(pipe(pipes) == -1){
			fprintf(stdout, "cannot create pipe\n");
		}

		//link_pipe(STDOUT_FILENO, pipes[WRITE]);
		buffer = malloc( sizeof(char) * (CMD_OUT_BUFF_SIZE + 1) );
		if(buffer == NULL){
			fprintf(stdout, "cannot allocate output buffer\n");
		}


		outf = open(out_pathname, O_WRONLY | O_CREAT, 0755);
		if(outf == -1){
			fprintf(stdout, "cannot create %s\n", out_pathname);
		}

		fprintf(stdout, "after stdout buffer allocation\n");

		ssize_t readb, writtenb;

		printf("linking stdout (%i) to pipes[WRITE](%i)\n", STDOUT_FILENO, pipes[WRITE]);
		printf("the father will read from pipes[READ] (%i) putting in buffer\n", pipes[READ]);
		printf("will write to outf (%i) from buffer\n", outf);
		printf("then will write to pipe out (%i) from buffer\n", pipe_out);
		link_pipe(STDOUT_FILENO, pipes[WRITE]);
		pid_t pid = fork();
		if(pid > 0){ //father

			char str[32];
			int c;
			for( c = 0; c < 10; c++ ){
				readb = read(pipes[READ], buffer, CMD_OUT_BUFF_SIZE);
				sprintf(str, "c:%i read:%zi\n", c, readb);
				write(outf, str, 32);

				if(readb > 0){ //se sono stati letti readb bytes
					//scrive nel log
					writtenb = write(outf, buffer, (size_t)readb);
					//scrive nella pipe out
					//!!!!!!! DEBUg stdout deve essere pipe_out
					writtenb = write(pipe_out, buffer, (size_t)readb);
				}

			}
		}
		else if(pid == 0){ //child

			execvp(cmd, args);
			printf("exec failed\n");
			exit(EXIT_FAILURE);
		}

	}




	exit(EXIT_SUCCESS);
}
