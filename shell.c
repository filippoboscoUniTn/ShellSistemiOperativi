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
//divided into many argument classes
#define S_ARG_STDOUTFILE 10
#define L_ARG_STDOUTFILE 1000
#define S_ARG_STDERRFILE 20
#define L_ARG_STDERRFILE 2000
#define S_ARG_MAXLEN 30
#define L_ARG_MAXLEN 3000

#define ARG_CODE_ERROR -1

void printUsage( char* _prog_name ){
	printf("usage: %s [-o | --outfile <file>] [-e | --errfile <file>] [-m | --maxlen <value>]\n	", _prog_name);
	exit(EXIT_SUCCESS);
}

//function to dinamically return arguments codes
int getcode(char *_arg){
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

	int curr_arg; //for loop argument counter
	char *t_arg; //temporary argument holder
	int arg_code; //argument code holder

	//arguments variables
	char *stdout_filepath;
	char *stderr_filepath;
	int max_len;

	curr_arg = 1;//argv[0] always executable name

	while( curr_arg < argc ){
		t_arg = argv[curr_arg];
		arg_code = getcode(t_arg);

		printf("t_arg: %s\n", t_arg);
		printf("arg_code: %i\n", arg_code);

		switch(arg_code){

			case L_ARG_STDOUTFILE:
			case S_ARG_STDOUTFILE:
				curr_arg++;
				stdout_filepath = argv[curr_arg];
				printf("stdout filepath: %s\n", stdout_filepath);
				break;

			case L_ARG_STDERRFILE:
			case S_ARG_STDERRFILE:
				curr_arg++;
				stderr_filepath = argv[curr_arg];
				printf("stderr filepath: %s\n", stderr_filepath);
				break;

			case L_ARG_MAXLEN:
			case S_ARG_MAXLEN:
				curr_arg++;
				max_len = atoi(argv[curr_arg]);
				printf("max_len: %i\n", max_len);
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
		curr_arg++;
	}

	printf("outfile: %s\n", stdout_filepath);
	printf("errfile: %s\n", stderr_filepath);
	printf("max_len: %i\n", max_len);

	exit(EXIT_SUCCESS);

}
