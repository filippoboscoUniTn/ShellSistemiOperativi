/*
	Shell source code.
	It listen from commands expressions from stdin and executes them.
	Through parameters it's possible to modify its behaviour:
		- where to save stdout file
		- where to save stderr file
		- where to save unique output file
		- options for selecting which informations have to be printed in the logfiles
	if one logfile file name is specified, the corresponding output is saved, otherwise not
	each process informations option is assumed true, false if specified
	If no parameters specified, it logs nothing, just executes
*/

#include "libs/std_libraries.h"
#include "libs/macros.h"
#include "libs/types.h"
#include "libs/functions.h"

//flag for the shell, it continues to listen until this flag it's put FALSE
volatile sig_atomic_t interrupted = FALSE; //put FALSE by the sigint_handler (when the user type the character sequence)

//customized SIGINT handler
void sigint_handler(int signum){

	interrupted = TRUE;
	return;
}

//main logic of the shell
int main(int argc, char **argv){

	//-------------------------------- SIGCHLD HANDLER REPLACEMENT --------------------------------
	struct sigaction act; //structure to be passed to the sigaction
	//act.sa_flags = SA_SIGINFO; //tells the sigaction that we want to execute sigaction and not sighandler
	sigemptyset(&act.sa_mask); //initialize the bitmask
	act.sa_handler = sigint_handler; //tells the sigaction to execute our handler
	sigaction(SIGINT, &act, NULL); //tells the OS to execute our handler when receiving SIGCHLD
	//---------------------------------------------------------------------------------------------


	//----------------------------------- VARIABLES DEFINITIONS -----------------------------------
	int curr_arg; //for loop argument counter (for accessing argv)
	char *t_arg; //for referencing actual parameter
	argcode_t arg_code; //argument code holder
	argtype_t arg_type; //argument type holder
	char *env_var_key = NULL; //pointer for temporarily holding environment variable key to be passed to the setenv()
	char input[CMD_EXP_BUFF_SIZE]; //buffer for holding the expression
	char *buffer; //buffer for holding part of the expression while tokenizing by ';'
	char *args[3]; //arguments buffer for passing it to exec
	char *path; //needed for holding the modified PATH environment variable value to be passed to the setenv()
	char *buffer_path = NULL; //needed for catching actual PATH environment variable value
	pid_t pid; //needed when forking
	int finished_executing = FALSE;
	int quitted = FALSE;
	//---------------------------------------------------------------------------------------------


	//--------------------------------- PARAMETERS HANDLING start ---------------------------------
	//if there are parameters we must parse and handle them
	if(argc > 1){

		curr_arg = 1; //argv[0] always executable name

		//---------- PRINT USAGE ----------
		//supported only as first argument
		//if it compares to -h or --help we print usage and exit
		//if -h or --help are put after other arguments it's error
		if(strcmp(argv[curr_arg], ARG_HELP_SS) == 0 || strcmp(argv[curr_arg], ARG_HELP_SL) == 0){
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
		}

		//---------- PARSING CYCLE start ----------
		//cycle to parse and elaborate all arguments
		while( curr_arg < argc ){

			//obtaining current informations
			t_arg = argv[curr_arg]; //catching actual parameter
			arg_code = getcode(t_arg); //catching actual parameter's code
			arg_type = gettype(arg_code); //catching actual parameter's type


			//---------- ARGUMENT TYPE SWITCH ----------
			//based on the argument's code we know where/how to save the value
			switch(arg_type){

				//---------- STRING & INT TYPE ----------
				//string and int argument's type logic
				case ARG_TYPE_S:
				case ARG_TYPE_I:
					curr_arg++; //next arg to catch the value

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
					}
					//if every other argument
					else{
						env_var_key = get_envstring(arg_code, env_var_key); //catching actual parameter environment variable id
						setenv(env_var_key, argv[curr_arg], OVERWRITE); //setting the environment variable with the parameter's value
					}

					break;

				//---------- SWITCH TYPE ----------
				//switch argument's type logic
				case ARG_TYPE_N:
					env_var_key = get_envstring(arg_code, env_var_key); //catching actual parameter environment variable id
					setenv(env_var_key, EV_FALSE, OVERWRITE); //setting the environment variable with FALSE (if no parameter the variable is assumed TRUE)

					break;

				//---------- TYPE ERROR ----------
				//argument not recognized/supported
				case ARG_CODE_ERROR:
					exit(EXIT_FAILURE);
					break;

				//default kept for exception handler
				default:
					exit(EXIT_FAILURE);
					break;
			}

			curr_arg++; //in every case we advance to next arg
		}
		//---------- PARSING CYCLE end ----------

	}
	//---------------------------------- PARAMETERS HANDLING end ----------------------------------


	//---------------------------- COMMANDS EXPRESSIONS LISTENING start ---------------------------
	//reading commands expressions from stdin

	printf("Shell started :)\ntype '/quit' to exit.\n\n");

	while( !quitted ){

		finished_executing = FALSE;
		interrupted = FALSE;

		fflush(stdin); //flushing stream for cleaning the input
		fflush(stdout);
		printf("BoGosh> "); //prompt
		fgets(input, CMD_EXP_BUFF_SIZE, stdin); //saves the input in the buffer to be tokenized

		if( strcmp(input, "/quit\n") == 0 ){
			quitted = TRUE;
		}

		else{
			buffer = strtok(input, ";"); //gets the part of the input before the first ';' if present
			while(!interrupted && !finished_executing){
				while(buffer != NULL){

					//Rimozione del new line dalla stringa da passare al controller
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
						strcpy(args[0],CONTROLLER_EXEC_NAME);
						args[1] = malloc( sizeof(char)*(strlen(buffer)+1) );
						strcpy(args[1],buffer);
						args[2] = NULL; //null terminated vector

						//debug




						fflush(stdout);

						execvp(CONTROLLER_EXEC_PATH, args);

						//debug

						perror("Error calling controller");

						exit(EXIT_SUCCESS);
					}

					//---------- FATHER ----------
					else if(pid > 0){
						buffer = strtok(NULL, ";"); //catches the next commands expression
					}

					//---------- ERROR FORKING ----------
					else{

						exit(EXIT_FAILURE);
					}
				}
				finished_executing = TRUE;
			}

			wait(NULL); //waits all children to finish before requesting another input
			fflush(stdout); //forces all output to be printed
		}
	}; //listen for inputs from stdin until SIGINT

	//----------------------------- COMMANDS EXPRESSIONS LISTENING end ----------------------------


	//------------------------------------- FREEING RESOURCES -------------------------------------
	/*if(env_var_key != NULL){
		free(env_var_key);
	}

	if(buffer != NULL){
		free(env_var_key);
	}

	if(path != NULL){
		free(path);
	}

	int i;
	for(i = 0; i < 3; i++){
		if(args[i] != NULL){
			free(args[i]);
		}
	}*/
	//---------------------------------------------------------------------------------------------

	printf("\nExited successfully.\n\n");
	exit(EXIT_SUCCESS);

}