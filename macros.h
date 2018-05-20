//include guard
#ifndef MACROS__H_
#define MACROS__H_




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
#define ARG_TYPE_N 3 //argument type null

//0 < code < 1000 --> for string arguments values
#define ARG_STDOUTFILE_C 10
#define ARG_STDOUTFILE_SS "-out\0"
#define ARG_STDOUTFILE_SL "--outfile\0"
#define ARG_STDERRFILE_C 20
#define ARG_STDERRFILE_SS "-err\0"
#define ARG_STDERRFILE_SL "--errfile\0"

//1000 < code < 2000 --> for integer arguments values
#define ARG_MAXLEN_C 1010
#define ARG_MAXLEN_SS "-m\0"
#define ARG_MAXLEN_SL "--maxlen\0"

//2000 < code < 3000 --> for null arguments values (only switches)
#define ARG_SWITCH_C 2010

//for not defined arguments
#define ARG_CODE_ERROR -1

//definition of prefixes for saving partial logs
//temp_pathname/LOG_PATH/cmd_name.txt (i.e. 123/out/ls.txt, 123/err/ls.txt)
#define OUT_LOG_PATH "out/"
#define ERR_LOG_PATH "err/"
#define PINFO_LOG_PATH "proc_info/"

//definition of environment variables
#define EV_TMP_PATH "temp_pathname"
#define EV_STDOUTFILE "stdout_filepath"
#define EV_STDERRFILE "stderr_filepath"
#define EV_PINFO_OUTFILE "proc_info_filepath"
#define EV_MAXLEN "max_output_length"
#define EV_PIPE_IN "pipe_in"
#define EV_PIPE_OUT "pipe_out"

//pipe macros
//#define PIPE_STDOUT 0
//#define PIPE_STDERR 1
#define READ 0
#define WRITE 1

#define BUFF_STDOUT 0
#define BUFF_STDERR 1

//commands name buffer size
#define CMD_NAME_BUFF_SIZE 32
//commands expression buffer size
#define CMD_EXP_BUFF_SIZE 512

#define CMD_OUT_BUFF_SIZE 1024





#endif
