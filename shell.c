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

#include "std_libraries.h"
#include "macros.h"
#include "types.h"
#include "functions.h"

//main logic of the shell
int main(int argc, char **argv){

	//-------------------------------- PARAMETERS HANDLING start --------------------------------
	//if there are parameters we must parse and handle them
	if(argc > 1){
		int curr_arg; //for loop argument counter (for accessing argv)
		char *t_arg; //for referencing actual parameter
		argcode_t arg_code; //argument code holder
		argtype_t arg_type; //argument type holder
		char *env_var_key; //pointer for temporarily holding environment variable key to be passed to the setenv()

		curr_arg = 1; //argv[0] always executable name

		//---------- PARSING CYCLE start ----------
		//cycle to parse and elaborate all arguments
		while( curr_arg < argc ){

			//obtaining current informations
			t_arg = argv[curr_arg]; //catching actual parameter
			arg_code = getcode(t_arg); //catching actual parameter's code
			arg_type = gettype(arg_code); //catching actual parameter's type

			printf("t_arg: %s\n", t_arg);
			printf("arg_code: %i\n", arg_code);
			printf("arg_type: %i\n", arg_type);

			//---------- ARGUMENT TYPE SWITCH ----------
			//based on the argument's code we know where/how to save the value
			switch(arg_type){

				//---------- STRING & INT TYPE ----------
				//string and int argument's type logic
				case ARG_TYPE_S:
				case ARG_TYPE_I:
					curr_arg++; //next arg to catch the value
					env_var_key = get_envstring(arg_code, env_var_key); //catching actual parameter environment variable id
					setenv(env_var_key, argv[curr_arg], OVERWRITE); //setting the environment variable with the parameter's value

					//debug
					printf("Set '%s' = '%s'\n", env_var_key, argv[curr_arg]);

					break;

				//---------- SWITCH TYPE ----------
				//switch argument's type logic
				case ARG_TYPE_N:
					env_var_key = get_envstring(arg_code, env_var_key); //catching actual parameter environment variable id
					setenv(env_var_key, EV_FALSE, OVERWRITE); //setting the environment variable with FALSE (if no parameter the variable is assumed TRUE)

					//debug
					printf("Set '%s' = '%s'\n", env_var_key, EV_FALSE);
					break;

				//---------- TYPE ERROR ----------
				//argument not recognized/supported
				case ARG_CODE_ERROR:
					printf("Error.\nIncorrect argument: %s\n", argv[curr_arg]);
					exit(EXIT_FAILURE);
					break;

				//default kept for exception handler
				default:
					printf("default??\n");
					printf("code: %i\n", arg_code);
					break;
			}

			curr_arg++; //in every case we advance to next arg
		}
		//---------- PARSING CYCLE end ----------

	}
	//-------------------------------- PARAMETERS HANDLING end --------------------------------

	//-------------------------------- COMMANDS EXPRESSIONS LISTENING start --------------------------------
	//reading commands expressions from stdin

	char input[CMD_EXP_BUFF_SIZE]; //buffer for holding the expression

	printf("Shell started.\ntype '/quit' to exit.\n\n");

	do{
		fflush(stdin); //flushing stream for cleaning the input
		printf("> "); // prompt
		fgets(input, CMD_EXP_BUFF_SIZE, stdin); // saves the input in the buffer to be tokenized
		//forse la tokenizza prima di passarla al controller
		//si forka: figlio --> exec controller passandogli i parametri     padre --> fa una wait e gestisce qualche errore, poi si rimette in ascolto
		printf("--> %s", input); //tanto per vedere se funzionava

	} while( strcmp(input, "/quit\n") != 0 ); //anche qua possiamo cambiare stringa/sequenza di escape

	//-------------------------------- COMMANDS EXPRESSIONS LISTENING end --------------------------------

	printf("\nExited succesfully.\n");
	exit(EXIT_SUCCESS);

}
