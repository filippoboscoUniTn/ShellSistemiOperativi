//include guard
#ifndef MACROS__H_
#define MACROS__H_


#define FALSE 0
#define TRUE 1

//ARGUMENTS CODE DEFINITION
//ending with C --> argument's code definition
//ending with S --> argument's string definition
//	ending with SS --> short arg, single hyphen (-out)
//	ending with SL --> long arg, double hyphen (--outfile)
//divided into many argument classes for keep the possibility to add other arguments
//and for separing logics, there are three types of arguments value: null, int or string
//separing arguments into numerics grops allow us to write less code and organize it

#define ARG_TYPE_S 1 //argument type string
#define ARG_TYPE_I 2 //argument type int
#define ARG_TYPE_N 3 //argument type switch (null)
#define ARG_TYPE_E -1 //argument type error

//0 < code < 1000
//argument type string
#define ARG_STDOUTFILE_C 10
#define ARG_STDOUTFILE_SS "-out\0"
#define ARG_STDOUTFILE_SL "--outfile\0"
#define ARG_STDERRFILE_C 20
#define ARG_STDERRFILE_SS "-err\0"
#define ARG_STDERRFILE_SL "--errfile\0"

//1000 < code < 2000
//argument type int
#define ARG_MAXLEN_C 1010
#define ARG_MAXLEN_SS "-m\0"
#define ARG_MAXLEN_SL "--maxlen\0"

//2000 < code < 3000
//argument type switch
#define ARG_SWITCH_C 2010

//code > 3000
//used as guard for catching errors
#define ARG_GUARD_C 3010

//for not defined arguments
#define ARG_CODE_ERROR -1

//definition of prefixes for saving partial logs
//needed by the controller to build file pathnames to pass to the logger via environment variables
//temp_pathname/LOG_PATH/cmd_name.txt (i.e. 123/out/ls.txt, 123/err/ls.txt)
#define OUT_LOG_PATH "out/"
#define ERR_LOG_PATH "err/"
#define PINFO_LOG_PATH "proc_info/"

//definition of environment variables
//needed by the logger when doing getenv()
//to get file pathnames and various switches from environment variables set by the controller
#define EV_TMP_PATH "temp_pathname"
#define EV_STDOUTFILE "stdout_filepath"
#define EV_STDERRFILE "stderr_filepath"
#define EV_PINFO_OUTFILE "proc_info_filepath"
#define EV_MAXLEN "max_output_length"
#define EV_PIPE_IN "pipe_in"
#define EV_PIPE_OUT "pipe_out"

//needed when doing setenv()
#define KEEP 0
#define OVERWRITE 1

//pipe macros
#define READ 0
#define WRITE 1


//commands name buffer size
#define CMD_NAME_BUFF_SIZE 32
//commands expression buffer size
#define CMD_EXP_BUFF_SIZE 512
//command output buffer size
//used by the logger to buffer the output, write it to the log and then to the output pipe
#define CMD_OUT_BUFF_SIZE 1024





#endif
