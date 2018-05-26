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

//commands expression buffer size
#define CMD_EXP_BUFF_SIZE 512

//Pipes' read and write ends
#define READ 0
#define WRITE 1

//Boolean TRUE and FALSE definition
#define TRUE 1
#define FALSE 0

//processIOtable parameters' MACROS
#define MAX_ARGUMENTS 32 //Numero massimo di argomenti per un comando
#define MAX_ARG_LEN 128 //Lunghezza massima degli argomenti di un comando
#define MAX_FILE_NAME_LEN 32 //Lunghezza massima della stringa che specifica un file su cui redirigere l'input di un comando
#define MAX_CMD_LEN 32

//Tokens MACROS

//Types
#define OPERATOR 100
#define COMMAND 200
#define OPTION 300
#define FILE 400

//Values
#define PIPE 0
#define AND 1
#define OR 2
#define IN_REDIRECT 3
#define OUT_REDIRECT 4

//Wrapper functions ERROR definition
#define ERR_PIPE 1
#define ERR_MKSTEMP 2
#define ERR_SETENV 3
#define ERR_SETENV_I 4
#define ERR_WAIT 5
#define ERR_OPEN 6






#endif
