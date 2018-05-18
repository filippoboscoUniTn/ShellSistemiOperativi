#include "std_libraries.h"
#include "functions.h"
#include "macros.h"
#include "types.h"

//function for keep the main logic clean and because it can be called multiple times
void printUsage( char* _prog_name ){
	printf("usage: %s [-o | --outfile <file>] [-e | --errfile <file>] [-m | --maxlen <value>]\n	", _prog_name);
	exit(EXIT_SUCCESS);
}


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

argtype_t gettype(argcode_t _argcode){

	//type string
	if(_argcode > 0 && _argcode < ARG_MAXLEN_C){
		return ARG_TYPE_S;
	}

	//type int
	if(_argcode >= ARG_MAXLEN_C && _argcode < ARG_SWITCH_C){
		return ARG_TYPE_I;
	}

	//type null
	else{
		return ARG_TYPE_N;
	}
}
