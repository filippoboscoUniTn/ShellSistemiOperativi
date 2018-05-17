#ifndef __STDIO_H__
#define __STDIO_H__
#include <stdio.h>
#endif

#ifndef __STDLIB_H__
#define __STDLIB_H__
#include <stdlib.h>
#endif

#ifndef __UNISTD_H__
#define __UNISTD_H__
#include <unistd.h>
#endif

#ifndef __STRING_H__
#define __STRING_H__
#include <string.h>
#endif

//ARGUMENTS CODE DEFINITION
//starting with S --> single hyphen arguments i.e. -o
//starting with L --> double hyphen arguments i.e. --outfile
//divided into many argument classes for keep the possibility to add other arguments
#define S_ARG_STDOUTFILE 10
#define L_ARG_STDOUTFILE 1000
#define S_ARG_STDERRFILE 20
#define L_ARG_STDERRFILE 2000
#define S_ARG_MAXLEN 30
#define L_ARG_MAXLEN 3000
//for not defined arguments
#define ARG_CODE_ERROR -1
//commands expression buffer size
#define CMD_EXP_BUFF_SIZE 512

typedef int argcode_t;
//argument type
typedef struct{
	argcode_t arg_code;//the argument's specific code for identification
	void *arg_value;//based on the argcode, this will be allocated for an int, char*...
} arg_t;

//function for keep the main logic clean and because it can be called multiple times
void printUsage( char* _prog_name ){
	printf("usage: %s [-o | --outfile <file>] [-e | --errfile <file>] [-m | --maxlen <value>]\n	", _prog_name);
	exit(EXIT_SUCCESS);
}

//function to dinamically return arguments codes
//matches the string with every argument
//returns error if the string doesn't match any argument
argcode_t getcode(char *_arg){
	if(strcmp(_arg, "-o\0") == 0){ return S_ARG_STDOUTFILE; }
	if(strcmp(_arg, "--outfile\0") == 0){ return L_ARG_STDOUTFILE; }
	if(strcmp(_arg, "-e\0") == 0){ return S_ARG_STDERRFILE; }
	if(strcmp(_arg, "--errfile\0") == 0){ return L_ARG_STDERRFILE; }
	if(strcmp(_arg, "-m\0") == 0){ return S_ARG_MAXLEN; }
	if(strcmp(_arg, "--maxlen\0") == 0){ return L_ARG_MAXLEN; }

	return ARG_CODE_ERROR;
}

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

	/*
	//arguments variables
	char *stdout_filepath;
	char *stderr_filepath;
	int max_len;
	*/

	curr_argv = 1;//argv[0] always executable name
	curr_args = 0;

	//cycle to parse and elaborate all arguments
	while( curr_argv < argc ){

		t_arg = argv[curr_argv]; //catching actual parameter
		arg_code = getcode(t_arg); //catching actual parameter's code
		char *str_value;
		int *int_value;

		printf("t_arg: %s\n", t_arg);
		printf("arg_code: %i\n", arg_code);

		//based on the argument's code we know where/how to save the value
		switch(arg_code){

			case L_ARG_STDOUTFILE:
			case S_ARG_STDOUTFILE:
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


			case L_ARG_STDERRFILE:
			case S_ARG_STDERRFILE:
				curr_argv++; //next arg to catch the value
				args[curr_args].arg_code = arg_code;
				str_value = malloc( strlen(argv[curr_argv]) + 1 );
				strcpy(str_value, argv[curr_argv]);
				args[curr_args].arg_value = str_value;

				str_value = NULL;
				curr_args++;
				break;

			case L_ARG_MAXLEN:
			case S_ARG_MAXLEN:
				curr_argv++; //next arg to catch the value
				args[curr_args].arg_code = arg_code;
				int_value = malloc( sizeof(int) );
				*int_value = atoi(argv[curr_argv]);
				args[curr_args].arg_value = int_value;

				int_value = NULL;
				curr_args++;
				break;

			case ARG_CODE_ERROR:
				printf("incorrect argument! \n aborting (to handle interactive mode) \n");
				printf("code: %i\n", arg_code);
				exit(EXIT_FAILURE);
				break;

			default:
				printf("default??\n");
				printf("code: %i\n", arg_code);
				break;
		}

		curr_argv++; //in every case we advance to next arg
	}

	/*
	printf("outfile: %s\n", stdout_filepath);
	printf("errfile: %s\n", stderr_filepath);
	printf("max_len: %i\n", max_len);
	printf("\n");
	*/

	int i; i=0;
	printf("before for....\n");
	for(i = 0; i < curr_args; i++){
		printf("args[%i] --> code: %i | ", i, args[i].arg_code);
		if(args[i].arg_code < S_ARG_MAXLEN){
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
