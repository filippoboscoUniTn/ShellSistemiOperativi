//include guard
#ifndef TYPES__H_
#define TYPES__H_




//argcode type definition
typedef int argcode_t;

//argument type definition
typedef int argtype_t;

//argument type
typedef struct{
	argcode_t arg_code; //the argument's specific code for identification
	void *arg_value; //based on the argcode, this will be allocated for an int, char* or whatever
} arg_t;

typedef struct{

	//these variables are nearly always used
	//based on the arguments switch, we know if allocate memory or not
	char *out_pathname; //stdout partial log file pathname
	char *err_pathname; //stderr partial log file pathname
	char *proc_info_pathname; //proc_info partial log file pathname
	int pipe_in; //input pipe read end which has to be linked to stdin of the executed command
	int pipe_out; //output pipe write end in which the logger redirect stdout of the executed program

	int outf; //stdout partial log file's FD
	int errf; //stderr partial log file's FD
	int proc_infof; //proc_info partial log file's FD

}loginfo_t;






#endif
