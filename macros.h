//include guard
#ifndef MACROS__H_
#define MACROS__H_


//Boolean TRUE and FALSE definition
#define TRUE 1
#define FALSE 0


//----------------------------------------------- ARGUMENTS CODE DEFINITION -----------------------------------------------
//arguments divided into many argument classes for keep the possibility to add other arguments
//and for separing logics, there are three types of arguments value: null, int or string
//separing arguments into numerics groups allow us to write less code and organize it

//------------------------- TYPES -------------------------
#define ARG_TYPE_S 1 //argument type string
#define ARG_TYPE_I 2 //argument type int
#define ARG_TYPE_N 3 //argument type switch (null)
#define ARG_TYPE_E -1 //argument type error

//-------------------- CODES & STRINGS --------------------
//ending with C --> argument's code definition
//ending with S --> argument's string definition
	//ending with SS --> short arg, single hyphen (-out)
	//ending with SL --> long arg, double hyphen (--outfile)

//------ STRING TYPE ------
//0 < code < 1000
#define ARG_STDOUTFILE_C 10
#define ARG_STDOUTFILE_SS "-out\0"
#define ARG_STDOUTFILE_SL "--outfile\0"
#define ARG_STDERRFILE_C 20
#define ARG_STDERRFILE_SS "-err\0"
#define ARG_STDERRFILE_SL "--errfile\0"
#define ARG_UNIOUTFILE_C 30
#define ARG_UNIOUTFILE_SS "-o\0"
#define ARG_UNIOUTFILE_SL "--output\0"
#define ARG_PATH_C 40
#define ARG_PATH_SS "-p\0"
#define ARG_PATH_SL "--path\0"

//help argument definition
//doesn't have a code because it's supported only as first argument
#define ARG_HELP_SS "-h\0"
#define ARG_HELP_SL "--help\0"

//------ INT TYPE ------
//1000 < code < 2000
#define ARG_MAXLEN_C 1010
#define ARG_MAXLEN_SS "-m\0"
#define ARG_MAXLEN_SL "--maxlen\0"

//------ SWITCH TYPE ------
//2000 < code < 3000
#define ARG_PROC_INFO_C 2010
#define ARG_PROC_INFO_SL "--noinfo\0"
#define ARG_ERRNO_C 2020
#define ARG_ERRNO_SL "--noerrno\0"
#define ARG_PID_C 2030
#define ARG_PID_SL "--nopid\0"
#define ARG_UID_C 2040
#define ARG_UID_SL "--nouid\0"
#define ARG_STATUS_C 2050
#define ARG_STATUS_SL "--nostatus\0"
//------ TYPE GUARD------
//used for catching errors when getting argument's type
//code > 3000
#define ARG_GUARD_C 3010

//for not defined arguments
#define ARG_CODE_ERROR -1


//-------------------------------------------- LOGGING INFORMATIONS DEFINITION --------------------------------------------

//----------------- PARTIAL LOGS PREFIXES -----------------
//definition of prefixes for saving partial logs
//needed by the controller to build file pathnames to pass to the logger via environment variables
//temp_pathname/LOG_PATH/cmd_name.txt (i.e. 123/out/ls.txt, 123/err/ls.txt)
#define OUT_LOG_PATH "out/"
#define ERR_LOG_PATH "err/"
#define PINFO_LOG_PATH "proc_info/"

//-------------------- ENVIRONMENT VARIABLES STRINGS --------------------
//definition of environment variables
//needed for setting and getting environment variables (controller and logger)
//for file pathnames and various switches from the shell

#define EV_SHELL_STDOUTFILE "shell_stdoutfile"
#define EV_SHELL_STDERRFILE "shell_stderrfile"
#define EV_SHELL_UNIOUTFILE "shell_unioutfile"
#define EV_STDOUTFILE "stdout_filepath"
#define EV_STDERRFILE "stderr_filepath"
#define EV_PINFO_OUTFILE "proc_info_filepath" //used internally
#define EV_PATH "PATH"
#define EV_MAXLEN "max_output_length"
#define EV_PIPE_IN "pipe_in" //used internally
#define EV_PIPE_OUT "pipe_out" //used internally
#define EV_PROC_INFO "process_info"
#define EV_ERRNO "errno"
#define EV_PID "pid"
#define EV_UID "uid"
#define EV_STATUS "status"

#define EV_TRUE "TRUE\0"
#define EV_FALSE "FALSE\0"


//needed when doing setenv()
#define KEEP 0 //tells setenv() to don't overwrite the variable if present
#define OVERWRITE 1 //tells setenv() to write the variable, no matter if already existing

//pipe macros
#define READ 0 //pipe's read end always the first int passed
#define WRITE 1 //pipe's write end always the second int passed

#define CMD_NAME_BUFF_SIZE 32 //commands name buffer size
#define CMD_EXP_BUFF_SIZE 512 //commands expression buffer size

//used by the logger to buffer the output, write it to the log and then to the output pipe
#define CMD_OUT_BUFF_SIZE 1024 //command output buffer size


//---------------------- EXEC IMAGES MACROS -------------------------
#define LOGGER_EXEC_PATH "/home/filippo/gitRepos/ShellSistemiOperativi/logger"
#define CONTROLLER_EXEC_PATH "/home/filippo/gitRepos/ShellSistemiOperativi/controller"
#define LOGGER_EXEC_NAME "logger"
#define CONTROLLER_EXEC_NAME "controller"

//---------------------- EXEC IMAGES MACROS -------------------------
//---------------------------- END ----------------------------------


//----------------- PROCESS TABLE PARAMETER'S MACROS ----------------
#define MAX_ARGUMENTS 32 //Numero massimo di argomenti per un comando
#define MAX_ARG_LEN 128 //Lunghezza massima degli argomenti di un comando
#define MAX_FILE_NAME_LEN 32 //Lunghezza massima della stringa che specifica un file su cui redirigere l'input di un comando
#define MAX_CMD_LEN 32
//----------------- PROCESS TABLE PARAMETER'S MACROS ----------------
//---------------------------- END ---------------------------------


//-------------------- TOKENS MACROS DEFINITION --------------------
//Types
#define OPERATOR 100
#define COMMAND 200
#define OPTION 300
#define FILE_ 400
//Values
#define PIPE 0
#define AND 1
#define OR 2
#define IN_REDIRECT 3
#define OUT_REDIRECT 4
//-------------------- TOKENS MACROS DEFINITION --------------------
//---------------------------- END ---------------------------------


//-------------------- ERROR CODES DEFINITION --------------------
//Wrapper functions ERRORS definition
#define ERR_PIPE 1
#define ERR_MKSTEMP 2
#define ERR_SETENV 3
#define ERR_SETENV_I 4
#define ERR_WAIT 5
#define ERR_OPEN 6
//Controller's ERRORS definition
#define ERR_CMD_EXSTS 7
#define ERR_MAX_ARGS 8
#define ERR_OPT_EXSTS 9
#define ERR_FILE_XPCTD 10
#define ERR_EXEC_FAIL 11
#define ERR_FORK_FAIL 12
#define ERR_SYNTAX_OPERATORS 13
#define ERR_SYNTAX_UNKNOWN 14
#define ERR_INVCATION_CNT 15
#define ERR_SEEK_FAIL 16
#define ERR_READ_FAIL 17
#define ERR_WRITE_FAIL 18
#define ERR_INPUT_OVRITE 19//Errore nel caso un utente cerchi di ri definire la redirezione più di una volta (i.e. ls | wc < file.txt)
#define ERR_CLOSE_FD 20
#define ERR_OP_NOT_SUPPORTED 21

//------------------- ERROR CODES DEFINITION --------------------
//--------------------------- END -------------------------------


#endif
