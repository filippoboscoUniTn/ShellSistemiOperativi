#include "std_libraries.h"
#include "functions.h"
#include "macros.h"
#include "types.h"

//function to print help usage
//keeps the main logic clean and because it can be called multiple times
//called in argument error cases and when argument is -h
void print_usage( char* _prog_name ){
	printf("usage: %s [-o | --outfile <file>] [-e | --errfile <file>] [-m | --maxlen <value>]\n	", _prog_name);
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
	if(strcmp(_arg, ARG_UNIOUTFILE_SS) == 0){ return ARG_UNIOUTFILE_C; }
	if(strcmp(_arg, ARG_UNIOUTFILE_SL) == 0){ return ARG_UNIOUTFILE_C; }
	if(strcmp(_arg, ARG_PATH_SS) == 0){ return ARG_PATH_C; }
	if(strcmp(_arg, ARG_PATH_SL) == 0){ return ARG_PATH_C; }

	if(strcmp(_arg, ARG_MAXLEN_SS)     == 0){ return ARG_MAXLEN_C; }
	if(strcmp(_arg, ARG_MAXLEN_SL)     == 0){ return ARG_MAXLEN_C; }

	if(strcmp(_arg, ARG_PROC_INFO_SL)     == 0){ return ARG_PROC_INFO_C; }
	if(strcmp(_arg, ARG_ERRNO_SL)     == 0){ return ARG_ERRNO_C; }
	if(strcmp(_arg, ARG_PID_SL)     == 0){ return ARG_PID_C; }
	if(strcmp(_arg, ARG_UID_SL)     == 0){ return ARG_UID_C; }
	if(strcmp(_arg, ARG_STATUS_SL)     == 0){ return ARG_STATUS_C; }

	return ARG_CODE_ERROR;
}

//needed by the shell
//function to dinamically return arguments environment variables string identifiers
//matches the argument code with every string identifier
//returns NULL pointer if the code doesn't match any id
char* get_envstring(argcode_t _argcode, char *_destination){

	if(_destination != NULL){
		free(_destination);
		_destination = NULL;
	}

	if(_argcode == ARG_STDOUTFILE_C){
		_destination = malloc( sizeof(char) * (strlen(EV_SHELL_STDOUTFILE)) );
		strcpy(_destination, EV_SHELL_STDOUTFILE);
	}

	if(_argcode == ARG_STDERRFILE_C){
		_destination = malloc( sizeof(char) * (strlen(EV_SHELL_STDERRFILE)) );
		strcpy(_destination, EV_SHELL_STDERRFILE);
	}

	if(_argcode == ARG_UNIOUTFILE_C){
		_destination = malloc( sizeof(char) * (strlen(EV_SHELL_UNIOUTFILE)) );
		strcpy(_destination, EV_SHELL_UNIOUTFILE);
	}

	if(_argcode == ARG_PATH_C){
		_destination = malloc( sizeof(char) * (strlen(EV_PATH)) );
		strcpy(_destination, EV_PATH);
	}

	if(_argcode == ARG_MAXLEN_C){
		_destination = malloc( sizeof(char) * (strlen(EV_MAXLEN)) );
		strcpy(_destination, EV_MAXLEN);
	}

	if(_argcode == ARG_PROC_INFO_C){
		_destination = malloc( sizeof(char) * (strlen(EV_PROC_INFO)) );
		strcpy(_destination, EV_PROC_INFO);
	}

	if(_argcode == ARG_ERRNO_C){
		_destination = malloc( sizeof(char) * (strlen(EV_ERRNO)) );
		strcpy(_destination, EV_ERRNO);
	}

	if(_argcode == ARG_PID_C){
		_destination = malloc( sizeof(char) * (strlen(EV_PID)) );
		strcpy(_destination, EV_PID);
	}

	if(_argcode == ARG_UID_C){
		_destination = malloc( sizeof(char) * (strlen(EV_UID)) );
		strcpy(_destination, EV_UID);
	}

	if(_argcode == ARG_STATUS_C){
		_destination = malloc( sizeof(char) * (strlen(EV_STATUS)) );
		strcpy(_destination, EV_STATUS);
	}

	return _destination;
}

//given an argument code it returns the type associated
argtype_t gettype(argcode_t _argcode){

	//type string
	if(_argcode > 0 && _argcode < ARG_MAXLEN_C){ return ARG_TYPE_S; }

	//type int
	else if(_argcode >= ARG_MAXLEN_C && _argcode < ARG_PROC_INFO_C){ return ARG_TYPE_I; }

	//type switch
	else if(_argcode >= ARG_PROC_INFO_C && _argcode < ARG_GUARD_C){ return ARG_TYPE_N; }

	//argument type error
	else{ return ARG_TYPE_E; }
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

//strcpy wrapper
//copies _source to _destination
//allocates memory for _destination if not done previously
char* my_strcpy(char *_source, char *_destination){
	if(_source == NULL){ return NULL; }
	if(_destination == NULL){
		_destination = my_malloc(strlen(_source));
	}

	_destination = strcpy(_destination, _source);

	return _destination;
}

//initialization function for loginfo_t
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

//function for free resources used by the logger
//it's not sure that every variable has been allocated, so it performs checks
//it flushes pipes before closing them
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

	if(_loginfo -> outf != -1){ fsync(_loginfo -> outf); close(_loginfo -> outf); }
	if(_loginfo -> errf != -1){ fsync(_loginfo -> errf); close(_loginfo -> errf); }
	if(_loginfo -> proc_infof != -1){ fsync(_loginfo -> proc_infof); close(_loginfo -> proc_infof); }
	if(_loginfo != NULL) { free(_loginfo); }

	//debug
	//printf("Resources freed!\n");

	return;
}
