/*
	Command logger source code
	This program will be called from the controller for every command to be executed.
	Depending on the environmental variables, it logs the stdout/stderr of the command and redirects it to the output pipe.
	Tt reads from the environment: files pathnames in which to log, pipe in/out and options switches (for what informations to log).
	The logger will behave as following:
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

//---------- DIRECTIVES ----------
#include "libs/macros.h"
#include "libs/std_libraries.h"
#include "libs/types.h"
#include "libs/functions.h"
//--------------------------------


//---------------------------------------------- SIGCHLD HANDLER & FLAG ------------------------------------------------
//the logger continues to log until this flag it's put FALSE
volatile sig_atomic_t proc_is_running = TRUE; //put FALSE by the sigchld_handler (when the executed command terminates)
volatile siginfo_t *pinfo = NULL; //structure where the handler saves process' informations (PID, UID, and so on)

//customized SIGCHLD handler
//it tells the logger to stop logging and  gives him a reference to process' informations
void sigchld_handler(int signum, siginfo_t *info, void *ucontext){

	proc_is_running = FALSE; //unlocking father
	pinfo = info; //giving father the reference

	return;
}
//---------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------- MAIN LOGIC ------------------------------------------------------
int main(int argc, char **argv){

	//if no arguments, error
	if(argc < 2){

		exit(EXIT_FAILURE);
	}

	//-------------------------------- SIGCHLD HANDLER REPLACEMENT --------------------------------
	struct sigaction act; //structure to be passed to the sigaction
	act.sa_flags = SA_SIGINFO; //tells the sigaction that we want to execute sigaction and not sighandler
	sigemptyset(&act.sa_mask); //initialize the bitmask
	act.sa_sigaction = sigchld_handler; //tells the sigaction to execute our handler
	sigaction(SIGCHLD, &act, NULL); //tells the OS to execute our handler when receiving SIGCHLD
	//---------------------------------------------------------------------------------------------


	//----------------------------------- VARIABLES DEFINITIONS -----------------------------------
	char *cmd; //command's name to be passed to the exec

	//allocating argc elements of type char*
	//argc - 1 for holding all the parameters + 1 for NULL pointer
	char *args[argc]; //command's arguments to be passed to the exec

	pid_t pid; //needed when forking
	loginfo_t *loginfo; //structure for holding logging informations passed in the environment by the controller
	int pipes[2]; //buffer pipe
	char *buffer = NULL; //buffer for temporarily holding (a part) of the executed command's stdout
	ssize_t readb, writtenb; //needed for saving return values of read() and write() syscalls

	char *env_buffer; //buffer for temporarily holding the result of getenv()
	FILE *proc_infof; //file stream for writing the process information
	int child_exit_status; //needed for exiting, the logger exits with the same status of the executed command (for notify the controller)
	//---------------------------------------------------------------------------------------------


	//--------------------------------- VARIABLES INITIALIZATIONS ---------------------------------
	catch_args(argc, argv, args); //removes logger name from argv, putting the result in args, for passing it to exec
	cmd = my_malloc(strlen(args[0])); //allocating space for command's name
	strcpy(cmd, args[0]); //catching command's name

	loginfo = malloc(sizeof(loginfo_t)); //allocating memory for the structure
	loginfo_init(loginfo); //initializing structure

	pipes[READ] = -1; //read end used to read the executed command stdout, log and redirect it
	pipes[WRITE] = -1; //write end linked to the executed command, this allow us to read and elaborate the output
	//---------------------------------------------------------------------------------------------



	//------------------------------ ENVIRONMENT VARIABLES CATCHING -------------------------------
	//here we catch all the logging informations passed by the controller
	//given these, we know where and what to log

	loginfo -> out_pathname = getenv(EV_STDOUTFILE);
	loginfo -> err_pathname = getenv(EV_STDERRFILE);
	loginfo -> proc_info_pathname = getenv(EV_PINFO_OUTFILE);

	loginfo -> pipe_in = atoi(getenv(EV_PIPE_IN));
	loginfo -> pipe_out = atoi(getenv(EV_PIPE_OUT));
	//---------------------------------------------------------------------------------------------

	if(loginfo -> pipe_in == -1){
		loginfo -> pipe_in = STDIN_FILENO;
	}

	if(loginfo -> pipe_out == -1){
		loginfo -> pipe_out = STDOUT_FILENO;
	}



	//------------------------------------- LOGGER LOGIC start ------------------------------------
	//based on the environment variables (if they are present or not)
	//we know the filepath(s) of the log files, pipes's file descriptors and process informations switches

	//----------------------------------- STDERR HANDLING start -----------------------------------
	//if we don't have to log the stderr, we don't redirect anything
	//instead, if we have to log it, we redirect the executed command's stderr to the log's file descriptor
	if(loginfo -> err_pathname == NULL){
		dup2(STDOUT_FILENO, STDERR_FILENO);
	}
	if(loginfo -> err_pathname != NULL){
		loginfo -> errf = open(loginfo -> err_pathname, O_WRONLY | O_CREAT, 0755); //opens stderr partial log file's FD

		//debug

		link_pipe(STDERR_FILENO, loginfo -> errf); //links the executed command stderr to the logfile's FD
	}
	//----------------------------------- STDERR HANDLING end -------------------------------------


	//----------------------------------- STDOUT HANDLING start -----------------------------------
	//if we don't have to save the output we just redirect stdout to pipe_out
	//if we have to log the stdout, we redirect it to the output pipe
	if(loginfo -> out_pathname == NULL){

		//dup2(loginfo -> pipe_out, STDOUT_FILENO); //links the executed command stdout to pipe_out

		//---------- FORK ----------
		//child has to link pipes and execute the command
		//father has to wait
		pid = fork(); //pid needed for separing father-child logics

		//---------- CHILD ----------
		if(pid == 0){

			dup2(loginfo -> pipe_in, STDIN_FILENO); //links stdin to the pipe_it before calling exec
			dup2(loginfo -> pipe_out, STDOUT_FILENO); //links stdout to the pipe_out before calling exec

			execvp(cmd, args); //executes the command with given parameters

			fprintf(stdout, "Error: command not found.\n");
			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}

		//---------- FATHER ----------
		//it has to log the output
		else if(pid > 0){

			wait(NULL); //waits the child and then continues executing logic
		}

		//---------- ERROR FORKING ----------
		else{
			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}
	}

	//if we have to log the stdout
	//we redirect the stdout to the pipe
	//allocate the buffer
	//and open the file stream to the log file
	if(loginfo -> out_pathname != NULL){

		//---------- ERROR CREATING PIPE ----------
		//if error creating pipe exits
		if(pipe(pipes) == -1){

			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}


		//---------- ERROR ALLOCATING BUFFER ----------
		buffer = my_malloc( CMD_OUT_BUFF_SIZE ); //allocating space for buffer
		//if error allocating memory exits
		if(buffer == NULL){

			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}


		//---------- ERROR OPENING STDOUT OUTPUT FILE ----------
		loginfo -> outf = open(loginfo -> out_pathname, O_WRONLY | O_CREAT, 0755); //opens stdout partial log file's FD
		//if error opening log file exits
		if(loginfo -> outf == -1){
			//debug

			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}


		//---------- FORK ----------
		//child has to link pipes and execute the command
		//father has to log the output
		pid = fork(); //pid needed for separing father-child logics

		//---------- FATHER ----------
		//it has to log the output
		if(pid > 0){
			close(pipes[WRITE]); //doesn't need to write to this pipe

			//debug
			//char str[128];
			//int err;

			//while the child process is running, the father will read from pipe and log the output
			while( proc_is_running ){
				readb = read(pipes[READ], buffer, CMD_OUT_BUFF_SIZE); //reading the command's stdout from pipe

				//if we have read some bytes we write those bytes
				//first in the log, then in the out pipe
				if(readb > 0){
					writtenb = write(loginfo -> outf, buffer, (size_t)readb); //writes to logfile
					writtenb = write(loginfo -> pipe_out, buffer, (size_t)readb); //writes to pipe_out
				}
			}

			//if we are here means that the child finished, and it's possible that he wrote some bytes before finishing and after our last read
			//so we continue to read until the pipe it's empty
			while((readb = read(pipes[READ], buffer, CMD_OUT_BUFF_SIZE)) > 0){//reading the command's stdout from pipe
				writtenb = write(loginfo -> outf, buffer, (size_t)readb); //writes to logfile
				writtenb = write(loginfo -> pipe_out, buffer, (size_t)readb);//writes to pipe_out
			}

			close(pipes[READ]);
			close(loginfo -> pipe_in);
			close(loginfo -> pipe_out);

		}

		//---------- CHILD ----------
		else if(pid == 0){

			close(pipes[READ]); //doesn't need to read from this pipe
			link_pipe(STDIN_FILENO, loginfo -> pipe_in); //links stdin to the pipe_it before calling exec
			link_pipe(STDOUT_FILENO, pipes[WRITE]); //links stdout to the buffer pipe before calling exec

			execvp(cmd, args); //executes the command with given parameters

			fprintf(stdout, "Error: command not found.\n");
			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}

		//---------- ERROR FORKING ----------
		else{
			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}

	}
	//----------------------------------- STDOUT HANDLING end -------------------------------------


	//-------------------------------- PROCESS INFO HANDLING start --------------------------------
	/*
		if environment variable is defined open the output file
		for every env var, we check if print it to the outfile
	*/

	//if the process info pathname has been defined in the environment by the controller we have to write informations in it
	if(loginfo -> proc_info_pathname != NULL){

		//---------- ERROR OPENING PROCESS INFO OUTPUT FILE ----------
		proc_infof = fopen(loginfo -> proc_info_pathname, "w"); //opens process info partial log file stream


		//if error opening log file exits
		if(proc_infof == NULL){
			free_resources(cmd, args, argc, buffer, loginfo);
			exit(EXIT_FAILURE);
		}


		//---------- PROCESS INFORMATIONS PRINTING ---------
		//for each environmental variable that's not FALSE (so not touched or put TRUE) we print the associated information

		env_buffer = getenv(EV_PID);
		if(env_buffer == NULL){

			fprintf(proc_infof, "%-15s %i\n",FRM_PID ,(int)pinfo -> si_pid);
		}

		env_buffer = getenv(EV_UID);
		if(env_buffer == NULL){

			fprintf(proc_infof, "%-15s %i\n",FRM_UID ,(int)pinfo -> si_uid);
		}

		env_buffer = getenv(EV_STATUS);
		if(env_buffer == NULL){

			fprintf(proc_infof, "%-15s %i\n",FRM_STATUS ,pinfo -> si_status);
		}

		env_buffer = getenv(EV_ERRNO);
		if(env_buffer == NULL){

			fprintf(proc_infof, "%-15s %i\n",FRM_ERRNO ,pinfo -> si_errno);
		}

		fflush(proc_infof);
		fclose(proc_infof); //we close the file stream

		child_exit_status = (int)pinfo -> si_status;
	}

	//-------------------------------- PROCESS INFO HANDLING end ----------------------------------

	if (WIFEXITED(child_exit_status)){
        child_exit_status = WEXITSTATUS(child_exit_status);
	}



	//-------------------------------------- LOGGER LOGIC end -------------------------------------

	//if we are here means that the command has finished and the logger too, so we free used resources
	if(pipes[READ] != -1){ close(pipes[READ]); }
	free_resources(cmd, args, argc, buffer, loginfo);

	exit(child_exit_status);
}
