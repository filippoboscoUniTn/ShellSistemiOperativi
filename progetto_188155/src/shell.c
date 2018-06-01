/*
	Shell source code.
	It listen from commands expressions from stdin and executes them.
	Through parameters it's possible to modify its behaviour:
		- where to save stdout file
		- where to save stderr file
		- where to save unique output file
		- options for selecting which informations have to be printed in the logfiles
	If one logfile file name is specified, the corresponding output is saved, otherwise not
	Each process informations option is assumed true, false if specified
	If no parameters specified, it logs nothing, just executes
*/


//---------- DIRECTIVES ----------
#include "libs/std_libraries.h"
#include "libs/macros.h"
#include "libs/types.h"
#include "libs/functions.h"
//--------------------------------


//---------------------------------------------- SIGINT HANDLER & FLAG ------------------------------------------------
//the shell continues to listen until this flag it's put TRUE
volatile sig_atomic_t interrupted = FALSE; //put TRUE by the sigint_handler (when the user type the character sequence)

//customized SIGINT handler
void sigint_handler(int signum){

	interrupted = TRUE; //sets the flag to communicate the shell that the user wants to interrupt the command execution
	return;
}
//---------------------------------------------------------------------------------------------------------------------




//--------------------------------------------------- MAIN LOGIC ------------------------------------------------------
int main(int argc, char **argv){

	//-------------------------------------- SIGINT HANDLER REPLACEMENT ------------------------------------
	struct sigaction act; //structure to be set and passed to the sigaction
	sigemptyset(&act.sa_mask); //empties the bitmask to don't block signals
	act.sa_handler = sigint_handler; //tells the sigaction to execute our custom handler
	sigaction(SIGINT, &act, NULL); //tells the OS to execute our handler when receiving SIGCHLD
	//------------------------------------------------------------------------------------------------------


	//---------------------------------------- VARIABLES DEFINITIONS ---------------------------------------
	int curr_arg; //for loop argument counter (for accessing argv while parsing)
	char *t_arg; //for referencing actual parameter
	argcode_t arg_code; //argument code holder
	argtype_t arg_type; //argument type holder
	char *env_var_key = NULL; //pointer for temporarily holding environment variable key to be passed to the setenv()
	char input[CMD_EXP_BUFF_SIZE]; //buffer for holding the user input expression
	char *buffer; //buffer for holding part of the expression while tokenizing by ';'
	char *args[3]; //arguments buffer for passing it to exec (0 controller name, 1 command expression, 2 null)
	char *path; //needed for holding the modified PATH environment variable value to be passed to the setenv()
	char *buffer_path = NULL; //needed for catching actual PATH environment variable value
	pid_t pid; //needed when forking
	int finished_executing = FALSE; //flag for stopping while loop when finished spawning controllers
	int quitted = FALSE; //flag for knowing when the user wants to quit the shell
	//------------------------------------------------------------------------------------------------------


	//------------------------------------- PARAMETERS HANDLING start --------------------------------------
	//if there are parameters we must parse and handle them
	//if not the shell starts taking user input
	if(argc > 1){

		curr_arg = 1; //argv[0] always executable name

		//---------- PRINT USAGE ----------
		//supported only as first argument
		//if it compares to -h or --help we print usage and exit
		//if -h or --help are put after other arguments it's considered error
		if(strcmp(argv[curr_arg], ARG_HELP_SS) == 0 || strcmp(argv[curr_arg], ARG_HELP_SL) == 0){
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
		}
		//--------------------------------


		//---------- PARSING CYCLE start ----------
		//cycle to parse and elaborate all arguments
		while( curr_arg < argc ){

			//obtaining current informations
			t_arg = argv[curr_arg]; //catching actual parameter
			arg_code = getcode(t_arg); //catching actual parameter's code
			arg_type = gettype(arg_code); //catching actual parameter's type


			//---------- ARGUMENT TYPE SWITCH start ----------
			//based on the argument's code we know where/how to save the value
			switch(arg_type){

				//---------- STRING & INT TYPE ----------
				//string and int argument's type logic
				case ARG_TYPE_S:
				case ARG_TYPE_I:
					curr_arg++; //next arg to catch the parameter's value

					//---------- PATH PARAMETER HANDLING ----------
					//when the user specify a PATH needs special handling for be set in the environment
					//has to be set in the first position for giving it priority respect the others
					if(arg_code == ARG_PATH_C){
						buffer_path = getenv(EV_PATH); //catching actual PATH value
						path = my_malloc(strlen(buffer_path) + 1 + strlen(argv[curr_arg])); //allocating space for holding the new PATH value
						path = my_strcpy(argv[curr_arg], path); //copying user's path into the vector for giving priority
						path = strcat(path, ":"); //adding the ':' separator
						path = strcat(path, buffer_path); //appending the rest of the PATH value

						setenv(EV_PATH, path, OVERWRITE); //setting all the values in the environment
						printf("set '%s' = '%s'\n", EV_PATH, path);
					}
					//if every other argument
					else{
						env_var_key = get_envstring(arg_code, env_var_key); //catching actual parameter environment variable id
						setenv(env_var_key, argv[curr_arg], OVERWRITE); //setting the environment variable with the parameter's value
						printf("set '%s' = '%s'\n", env_var_key, argv[curr_arg]);
					}

					break;

				//---------- SWITCH TYPE ----------
				//switch argument's type logic
				case ARG_TYPE_N:
					env_var_key = get_envstring(arg_code, env_var_key); //catching actual parameter environment variable id
					setenv(env_var_key, EV_FALSE, OVERWRITE); //setting the environment variable with FALSE (if no parameter the variable is assumed TRUE)
					printf("set '%s'\n", env_var_key);

					break;

				//---------- TYPE ERROR ----------
				//argument not recognized/supported
				case ARG_CODE_ERROR:
					printf("Error: argument '%s' not valid.\n", t_arg);
					exit(EXIT_FAILURE);

					break;

				//default kept for exception handler
				default:
					exit(EXIT_FAILURE);

					break;
			}
			//---------- ARGUMENT TYPE SWITCH end ----------

			curr_arg++; //in every case we advance to next arg
		}
		//---------- PARSING CYCLE end ----------

	}
	//------------------------------------- PARAMETERS HANDLING end --------------------------------------


	//-------------------------------- COMMANDS EXPRESSIONS LISTENING start ------------------------------
	//reading commands expressions from stdin

	printf("BoGo shell started.\nType '/quit' to exit.\n\n"); //welcome message and indication how to exit

	//when the user types '/quit' the flag is put TRUE
	//the shell exits the while loop and dies with no errors
	while( !quitted ){

		//resetting flags at every iteration of the while loop
		finished_executing = FALSE; //put TRUE when finished spawning controllers for exiting the spawning while loop
		interrupted = FALSE; //put TRUE when the user sends SIGINT to the process pressing the character sequence

		fflush(stdin); //flushing stream for cleaning the input
		fflush(stdout); //flushing stream for cleaning the output
		printf("BoGosh> "); //prompt
		fgets(input, CMD_EXP_BUFF_SIZE, stdin); //saves the input in the buffer to be tokenized


		//---------- QUITTING CHECK ----------
		if( strcmp(input, "/quit\n") == 0 ){
			quitted = TRUE;
		}
		//------------------------------------


		//---------- SPAWNING LOGIC start ----------
		else{
			buffer = strtok(input, ";"); //gets the part of the input before the first ';' if present

			//while user hasn't interrupted the execution, the shell continues spawning controllers
			//when finishes to spawn controllers it waits the termination of all children
			//then asks another input
			while(!interrupted && !finished_executing){

				//while there are ';' separated expressions continues to spawn loggers
				while(buffer != NULL){

					//removing newline from the string to be passed to the controller
					int bufferLen = strlen(buffer);
					if( bufferLen > 0 && buffer[bufferLen-1] == '\n'){buffer[bufferLen-1] = '\0';}

					//---------- FORK ----------
					//child will exec the controller
					//father will continue parsing and forking
					pid = fork();

					//---------- CHILD ----------
					if(pid == 0){

						//arguments vector to be passed to the exec
						args[0] = malloc(sizeof(char)*strlen(CONTROLLER_EXEC_NAME)+1);
						strcpy(args[0],CONTROLLER_EXEC_NAME); //first argument the controller's executable name
						args[1] = malloc( sizeof(char)*(strlen(buffer)+1) );
						strcpy(args[1],buffer); //second argument the command expression
						args[2] = NULL; //third argument null terminated strin

						fflush(stdout); //forcing stdout to be printed

						execvp(CONTROLLER_EXEC_PATH, args); //controller execution

						perror("Error calling controller"); //error

						exit(EXIT_FAILURE); //exits with error
					}

					//---------- FATHER ----------
					else if(pid > 0){
						buffer = strtok(NULL, ";"); //catches the next commands expression and continues tokenizing
					}

					//---------- ERROR FORKING ----------
					else{
						exit(EXIT_FAILURE); //exits with error
					}
				}
				finished_executing = TRUE; //set the flag TRUE for exiting the while loop
			}

			wait(NULL); //waits all children to finish before requesting another input
			fflush(stdout); //forces all output to be printed
		}
		//---------- SPAWNING LOGIC end ----------

	};

	//----------------------------- COMMANDS EXPRESSIONS LISTENING end ----------------------------


	//------------------------------------- FREEING RESOURCES -------------------------------------
	if(env_var_key != NULL){ free(env_var_key); }

	if(buffer != NULL){ free(buffer); }

	if(path != NULL){ free(path); }

	int i;
	for(i = 0; i < 3; i++){
		if(args[i] != NULL){
			free(args[i]);
		}
	}
	//---------------------------------------------------------------------------------------------

	printf("\nExited successfully.\n\n");
	exit(EXIT_SUCCESS);

}
