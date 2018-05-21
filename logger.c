/*
	CHANGES:
	- pensare a cosa deve essere singolarmente passato come parametro e cosa può essere passato come variabile d'ambiente
*/


/*
	this is the command logger source code
	this program will be called from the shell for every command to be executed
	depending on the options switch, it logs the stdout/stderr of the command and redirects it to the output pipe
	it reads from the environment: files pathnames in which to log, pipe in/out and options switches (for what informations to log)

	the logger will behave as following:
		1) catch the 1st arg treated as command's name
		2) catch the 2nd arg treated as command's parameters (if present)
		3) start scanning all environment variables for catching:
			- temporary folder pathname in which to save partial logs
			- shell's parameters' array for knowing what informations to log, and where to save it (options switches)
			- pipes's file descriptors
		4) based on the informations to save, it will open output stream files, get process informations, link pipes and allocate buffers
		5) fork --> child will execute the command
				--> father will read input pipes, log to files and then write to output pipes
		6) when the child finishes, the father flushes the buffers, closes output streams, notify the controller and then exit
*/

#include "macros.h"
#include "std_libraries.h"
#include "types.h"
#include "errno.h"
#include "functions.h"

//main logic of the logger
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
	cmd = my_malloc(strlen(args[0])); //allocating space for command's name
	strcpy(cmd, args[0]); //catching command's name



	//debug
	/*
	printf("cmd --> %s\n", cmd);
	int j;
	for(j = 0; args[j] != NULL; j++){
		printf("argv[%i] --> %s\n", j, args[j]);
	}

	printf("before strcpy\n");
	*/



	//these variables will be put in the environment by the controller
	char *t_out = my_malloc( 200 );
	strcpy(t_out, "/Users/nik/Desktop/stdout/");
	strcat(t_out, cmd);
	strcat(t_out, ".txt");
	char *t_err = my_malloc( 200 );
	strcpy(t_err, "/Users/nik/Desktop/stderr/");
	strcat(t_err, cmd);
	strcat(t_err, ".txt");

	//debug
	//printf("after strcpy\n");

	setenv(EV_MAXLEN, "200", OVERWRITE);
	setenv(EV_STDOUTFILE, t_out, OVERWRITE);
	setenv(EV_STDERRFILE, t_err, OVERWRITE);
	setenv(EV_PIPE_IN, "0", OVERWRITE);
	setenv(EV_PIPE_OUT, "1", OVERWRITE);


	//these variables are nearly always used
	//based on the arguments switch, we know if allocate memory or not
	char *out_pathname = NULL; //stdout partial log file pathname
	char *err_pathname = NULL; //stderr partial log file pathname
	char *proc_info_pathname = NULL; //proc_info partial log file pathname
	int pipe_in = -1; //input pipe read end which has to be linked to stdin of the executed command
	int pipe_out = -1; //output pipe write end in which the logger redirect stdout of the executed program

	int outf = -1; //stdout partial log file's FD
	int errf = -1; //stderr partial log file's FD
	int proc_infof = -1; //proc_info partial log file's FD

	int pipes[2]; //buffer pipe
	pipes[READ] = -1; //read end used to read the executed command stdout, log and redirect it
	pipes[WRITE] = -1; //write end linked to the executed command, this allow us to read and elaborate the output
	char *buffer; //buffer for temporarily holding (a part) of the executed command stdout

	//debug
	//printf("before getenvs and atois\n");

	//here we catch all environment variables needed
	//then we also know what to do
	pipe_in = atoi(getenv(EV_PIPE_IN));
	pipe_out = atoi(getenv(EV_PIPE_OUT));

	out_pathname = getenv(EV_STDOUTFILE);
	err_pathname = getenv(EV_STDERRFILE);
	proc_info_pathname = getenv(EV_PINFO_OUTFILE);


	//debug
	/*
	printf("after getenvs and atois\n");

	printf("outf_pathname --> %s\n", out_pathname);
	printf("errf_pathname --> %s\n", err_pathname);
	printf("proc_infof_pathname --> %s\n", proc_info_pathname);
	printf("pipe_in --> %i\n", pipe_in);
	printf("pipe_out --> %i\n", pipe_out);
	*/


	//if we don't have to log the stderr, we don't redirect anything
	//instead, if we have to log it, we redirect the executed command's stderr to the log's file descriptor
	if(err_pathname != NULL){
		//opens stderr partial log file's FD
		errf = open(err_pathname, O_WRONLY | O_CREAT, 0755);

		//debug
		//printf("after opening the errfile errf --> %i\n", errf);

		//links the executed command stderr to the logfile's FD
		link_pipe(STDERR_FILENO, errf);
	}

	//if we don't have to save the output we just redirect stdout to pipe_out
	if(out_pathname == NULL){

		//debug
		//printf("linked stdout to pipe_out\n");

		//links the executed command stdout to pipe_out
		link_pipe(STDOUT_FILENO, pipe_out);
	}

	//if we have to log the stdout
	//we redirect the stdout to the pipe
	//allocate the buffer
	//and open the file stream to the log file
	if(out_pathname != NULL){

		//debug
		//printf("before stdout buffer allocation\n");

		//if error creating pipe exits
		if(pipe(pipes) == -1){

			//debug
			//fprintf(stdout, "cannot create pipe\n");
			exit(EXIT_FAILURE);
		}

		//allocating space for buffer
		buffer = my_malloc( CMD_OUT_BUFF_SIZE );
		//if error allocating memory exits
		if(buffer == NULL){

			//debug
			//fprintf(stdout, "cannot allocate output buffer\n");
			exit(EXIT_FAILURE);
		}

		//opens stdout partial log file's FD
		outf = open(out_pathname, O_WRONLY | O_CREAT, 0755);
		//if error opening log file exits
		if(outf == -1){

			//debug
			//fprintf(stdout, "cannot create %s\n", out_pathname);

			exit(EXIT_FAILURE);
		}

		//debug
		//fprintf(stdout, "after stdout buffer allocation\n");

		ssize_t readb, writtenb;

		//debug
		/*
		printf("linking stdout (%i) to pipes[WRITE](%i)\n", STDOUT_FILENO, pipes[WRITE]);
		printf("the father will read from pipes[READ] (%i) putting in buffer\n", pipes[READ]);
		printf("will write to outf (%i) from buffer\n", outf);
		printf("then will write to pipe out (%i) from buffer\n", pipe_out);
		*/

		//forking
		//child has to link pipes and execute the command
		//father has to log the output
		pid_t pid = fork();

		//father
		if(pid > 0){

			close(pipes[WRITE]); //doesn't need to write to this pipe

			//debug
			//char str[128];
			//int err;

			int c;
			for( c = 0; c < 10; c++ ){
				readb = read(pipes[READ], buffer, CMD_OUT_BUFF_SIZE);

				//debug
				//err = errno;
				//sprintf(str, "c:%i read:%zi\n", c, readb);
				//write(outf, str, 128);

				//if we have read some bytes we write those bytes
				if(readb > 0){

					//writes to logfile
					writtenb = write(outf, buffer, (size_t)readb);
					//writes to pipe_out
					writtenb = write(pipe_out, buffer, (size_t)readb);
				}
				//debug
				//sleep(1);

			}
		}

		//child
		else if(pid == 0){

			close(pipes[READ]); //doesn't need to read from this pipe
			link_pipe(STDIN_FILENO, pipe_in); //links stdin to the pipe_it before calling exec
			link_pipe(STDOUT_FILENO, pipes[WRITE]); //links stdout to the buffer pipe before calling exec

			execvp(cmd, args); //executes the command with given parameters
			//debug
			//printf("exec failed\n");
			exit(EXIT_FAILURE);
		}

		//if error forking exits
		else{
			exit(EXIT_FAILURE);
		}

	}




	exit(EXIT_SUCCESS);
}
