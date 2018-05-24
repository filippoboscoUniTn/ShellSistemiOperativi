#include "std_libraries.h"
#include "functions.h"
#include "macros.h"
#include "types.h"

//function to print help usage
//keeps the main logic clean and because it can be called multiple times
//called in argument error cases and when argument is -h
void printUsage( char* _prog_name ){
	printf("usage: %s [-o | --outfile <file>] [-e | --errfile <file>] [-m | --maxlen <value>]\n	", _prog_name);
	exit(EXIT_SUCCESS);
}

//needed by the shell
//function to dinamically return arguments codes
//matches the string with every argument
//returns error if the string doesn't match any argument
argcode_t getcode(char *_arg){
	if(strcmp(_arg, ARG_STDOUTFILE_SS) == 0){ return ARG_STDOUTFILE_C; }
	if(strcmp(_arg, ARG_STDOUTFILE_SL) == 0){ return ARG_STDOUTFILE_C; }
	if(strcmp(_arg, ARG_STDERRFILE_SS) == 0){ return ARG_STDERRFILE_C; }
	if(strcmp(_arg, ARG_STDERRFILE_SL) == 0){ return ARG_STDERRFILE_C; }
	if(strcmp(_arg, ARG_MAXLEN_SS)     == 0){ return ARG_MAXLEN_C; }
	if(strcmp(_arg, ARG_MAXLEN_SL)     == 0){ return ARG_MAXLEN_C; }

	return ARG_CODE_ERROR;
}

//given an argument code it returns the type associated
argtype_t gettype(argcode_t _argcode){

	//type string
	if(_argcode > 0 && _argcode < ARG_MAXLEN_C){
		return ARG_TYPE_S;
	}

	//type int
	else if(_argcode >= ARG_MAXLEN_C && _argcode < ARG_SWITCH_C){
		return ARG_TYPE_I;
	}

	//type switch
	else if(_argcode >= ARG_SWITCH_C && _argcode < ARG_GUARD_C){
		return ARG_TYPE_N;
	}

	//argument type error
	else{
		return ARG_TYPE_E;
	}
}

//needed by the shell, controller and logger
//removes the argv[0] from the arguments matrix argv and puts the result in args
//argc is the same for both argv and args
//iterates copying from argv to args, skipping argv[0] and appending NULL to the end of args
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

//needed by the controlled logger
//links _source pipe to _destination pipe
//closes _destination because once redirected _source to the pipe pointed by _destination,
//the initial pointer (_destination) is no longer needed
void link_pipe(int _source, int _destination){
	dup2(_destination, _source); //redirects the file descriptor in the FD's table
	close(_destination); //closes the initial FD's pointer
	return;
}

//malloc wrapper
//can be used only for char*
//allocates _n char position + 1 for holding termination string char
char* my_malloc(int _n){
	char *mem_ref; //memory reference
	mem_ref = malloc( sizeof(char) * (_n + 1) );
	return mem_ref;
}

char* my_strcpy(char *_source, char *_destination){
	if(_source == NULL){ return NULL; }
	if(_destination == NULL){
		_destination = my_malloc(strlen(_source));
	}

	_destination = strcpy(_destination, _source);

	return _destination;
}

void loginfo_init(loginfo_t *_loginfo){

	_loginfo -> out_pathname = NULL;
	_loginfo -> err_pathname = NULL;
	_loginfo -> proc_info_pathname = NULL;
	_loginfo -> pipe_in = -1;
	_loginfo -> pipe_out = -1;

	_loginfo -> outf = -1;
	_loginfo -> errf = -1;
	_loginfo -> proc_infof = -1;


	return;
}

void free_resources(char *_cmd, char ** _args, int _argc, char *_buffer, loginfo_t *_loginfo){
	//debug
	//printf("Starting freeing resources..........\n...........\n");

	if(_cmd != NULL){
		free(_cmd);
	}

	int i;
	for(i = 0; i < _argc; i++){
		if(_args[i] != NULL){
			free(_args[i]);
		}
	}

	if(_buffer != NULL){
		free(_buffer);
	}

	//if(_loginfo -> pipe_in != -1){ close(_loginfo -> pipe_in); }
	//if(_loginfo -> pipe_out != -1){ close(_loginfo -> pipe_out); }

	if(_loginfo -> outf != -1){ close(_loginfo -> outf); }
	if(_loginfo -> errf != -1){ close(_loginfo -> errf); }
	if(_loginfo -> proc_infof != -1){ close(_loginfo -> proc_infof); }
	if(_loginfo != NULL) { free(_loginfo); }

	//debug
	//printf("Resources freed!\n");

	return;
}
