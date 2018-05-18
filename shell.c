#include "std_libraries.h"
#include "macros.h"
#include "types.h"
#include "functions.h"

//main logic of the shell
int main(int argc, char **argv){

	//here we check if arguments list is wrong
	//at least one output file name is needed, other arguments are optional
	if( argc < 2 ){ //first argument is always executable name
		printf("Not enough arguments: you need to specify at least one output file. \n");
		printUsage(argv[0]);
		exit(EXIT_SUCCESS);
	}


	//assuming the worst case, arguments are all on/off switches (without a value)
	//except one that for sure has to hold an output file name
	//so we allocate argc-1 positions in the vector (the first is always the executable name)
	arg_t args[argc-1]; //vector for saving parameters code and value
	int curr_argv; //for loop argument counter (for accessing argv)
	int curr_args; //for loop argument counter (for accessing args)
	char *t_arg; //temporary argument holder
	argcode_t arg_code; //argument code holder
	argtype_t arg_type; //argument type holder

	curr_argv = 1; //argv[0] always executable name
	curr_args = 0;

	//cycle to parse and elaborate all arguments
	while( curr_argv < argc ){

		t_arg = argv[curr_argv]; //catching actual parameter
		arg_code = getcode(t_arg); //catching actual parameter's code
		arg_type = gettype(arg_code); //catching actual parameter's type

		//pointers for the three types of parameters' values
		char *str_value; //string
		int *int_value; //integer
		//manca bool per switch

		printf("t_arg: %s\n", t_arg);
		printf("arg_code: %i\n", arg_code);

		//based on the argument's code we know where/how to save the value
		switch(arg_type){

			//string argument's value logic
			case ARG_TYPE_S:
				curr_argv++; //next arg to catch the value
				args[curr_args].arg_code = arg_code;
				str_value = malloc( strlen(argv[curr_argv]) + 1 );
				printf("before strcpy....\n");
				strcpy(str_value, argv[curr_argv]);
				printf("str_value: %s\n", str_value);
				args[curr_args].arg_value = str_value;
				printf("arg_value: %s\n", args[curr_args].arg_value);

				str_value = NULL;
				curr_args++;
				break;

			//int argument's value logic
			case ARG_TYPE_I:
				curr_argv++; //next arg to catch the value
				args[curr_args].arg_code = arg_code;
				int_value = malloc( sizeof(int) );
				*int_value = atoi(argv[curr_argv]);
				args[curr_args].arg_value = int_value;

				int_value = NULL;
				curr_args++;
				break;

			//manca la gestione dei singoli switch
			//allocare un puntatore a bool nel void*
			case ARG_TYPE_N:
				printf("ssssswwwwwiittcchhhh\n");
				break;

			//argument not recognized/supported
			case ARG_CODE_ERROR:
				printf("incorrect argument! \n aborting (to handle interactive mode) \n");
				printf("code: %i\n", arg_code);
				exit(EXIT_FAILURE);
				break;

			//default kept for exception handler
			default:
				printf("default??\n");
				printf("code: %i\n", arg_code);
				break;
		}

		curr_argv++; //in every case we advance to next arg
	}

	int i; i=0;
	printf("before for....\n");
	for(i = 0; i < curr_args; i++){
		printf("args[%i] --> code: %i | ", i, args[i].arg_code);
		if(args[i].arg_code < ARG_MAXLEN_C){
			printf("%s \n", args[i].arg_value);
		}
		else{
			printf("%i \n", *(int*)args[i].arg_value);
		}
	}

	//reading commands expressions from stdin logic
	char input[CMD_EXP_BUFF_SIZE];

	printf("Shell started.\ntype '/quit' to exit.\n\n");

	do{
		fflush(stdin); //flushing stream for cleaning the input
		printf("> "); // prompt
		fgets(input, CMD_EXP_BUFF_SIZE, stdin); // saves the input in the buffer to be tokenized
		printf("--> %s", input); //tanto per vedere se funzionava

	} while( strcmp(input, "/quit\n") != 0 ); //anche qua possiamo cambiare stringa/sequenza di escape

	printf("\nExited succesfully.\n");
	exit(EXIT_SUCCESS);

}
