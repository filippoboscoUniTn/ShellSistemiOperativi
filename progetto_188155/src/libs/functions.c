#include "std_libraries.h"
#include "functions.h"
#include "macros.h"
#include "types.h"

//function to print help usage
//keeps the main logic clean and because it can be called multiple times
//called in argument error cases and when argument is -h
void print_usage( char* _prog_name ){
	printf("usage: %s [option [value] ]\n\n", _prog_name);
	printf("Options can be used to modify shell's behaviour.\n");
	printf("Without defined options, the shell will only execute commands expressions.\n");
	printf("OPTIONS:\n\n");

	//-h | --help
	printf("[%s | %s] : displays this usage message.\n", ARG_HELP_SS, ARG_HELP_SL);

	//-out | --outfile
	printf("[%s | %s] <file> : defines the file in which to save commands standard output\n", ARG_STDOUTFILE_SS, ARG_STDOUTFILE_SL);
	//-err | --errfile
	printf("[%s | %s] <file> : defines the file in which to save commands standard error\n", ARG_STDERRFILE_SS, ARG_STDERRFILE_SL);
	//-o | --output
	printf("[%s | %s] <file> : defines the file in which to save both commands standard output and standard error\n", ARG_UNIOUTFILE_SS, ARG_UNIOUTFILE_SL);
	//-p | --path
	printf("[%s | %s] <path> : tells the shell where to look for searching other commands.\nMultiple paths are allowed, just separate them with ':'\n", ARG_PATH_SS, ARG_PATH_SL);
	//-m | --maxlen
	printf("[%s | %s] <value> : defines the max lenght of the command standard output/error written in the log file\nIf the command's output is longer than <value>, the output will be truncated.\n", ARG_MAXLEN_SS, ARG_MAXLEN_SL);
	//--noinfo
	printf("[%s] : tells the shell to don't print any command's process information in the log file.\n", ARG_PROC_INFO_SL);
	//--noerrno
	printf("[%s] : tells the shell to don't print command's errno in the log file.\n", ARG_ERRNO_SL);
	//--nopid
	printf("[%s] : tells the shell to don't print command's PID in the log file.\n", ARG_PID_SL);
	//--nouid
	printf("[%s] : tells the shell to don't print command's UID in the log file.\n", ARG_UID_SL);
	//--nostatus
	printf("[%s] : tells the shell to don't print command's status in the log file.\n\n", ARG_STATUS_SL);

	return;
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
		_destination = malloc( sizeof(char) * (strlen(EV_SHELL_STDOUTFILE)+1) );
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
	//

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
	//

	return;
}

//Error handling functions
char * getErrorMessage(int errno){
  char errMessage [MAX_CMD_LEN];
  switch (errno) {
    case ERR_PIPE:
    	strcpy(errMessage,"Error opening pipe\0");
    break;

    case ERR_MKSTEMP:
    	strcpy(errMessage,"Error creating temporary file\0");
    break;

    case ERR_OPEN:
    	strcpy(errMessage,"Error opening file\0");
    break;

    case ERR_WAIT:
    	strcpy(errMessage,"Error waiting for child\0");
    break;

    case ERR_SETENV:
    	strcpy(errMessage,"Error setting environmental variable for string value\0");
    break;

    case ERR_SETENV_I:
    	strcpy(errMessage,"Error setting environmental variable for integer value\0");
    break;

		case ERR_CMD_EXSTS:
			strcpy(errMessage,"Error copying command to process' table. Alredy a command\0");
		break;

		case ERR_MAX_ARGS:
			strcpy(errMessage,"Error : too many arguments\0");
		break;

		case ERR_OPT_EXSTS:
			strcpy(errMessage,"Error copying option to process' table. Alredy an option\0");
		break;

		case ERR_REDIR_FILEXPCTD:
			strcpy(errMessage,"Error : expected file after IN/OUT redirect operator\0");
		break;

		case ERR_EXEC_FAIL:
			strcpy(errMessage,"Error swaping process image\0");
		break;

		case ERR_FORK_FAIL:
			strcpy(errMessage,"Error spawning new child\0");
		break;

		case ERR_SYNTAX_OPERATORS:
			strcpy(errMessage,"Error : too many operators\0");
		break;

		case ERR_SYNTAX_UNKNOWN:
			strcpy(errMessage,"Unknows syntax error\0");
		break;

    default:
    	strcpy(errMessage,"Error recognizing error : errception\0");
  }
  return NULL;
}

void printError(int errno){
  switch (errno) {

    case ERR_PIPE:
    	printf("Error opening pipe\n");
    break;

    case ERR_MKSTEMP:
    	printf("Error creating temporary file\n");
    break;

    case ERR_OPEN:
    	printf("Error opening file\n");
    break;

    case ERR_WAIT:
    	printf("Error waiting for child\n");
    break;

    case ERR_SETENV:
    	printf("Error setting environmental variable for string value\n");
    break;

    case ERR_SETENV_I:
    	printf("Error setting environmental variable for integer value\n");
    break;

		case ERR_CMD_EXSTS:
			printf("Error copying command to process' table. Alredy a command\n");
		break;

		case ERR_MAX_ARGS:
			printf("Error : too many arguments\n");
		break;

		case ERR_OPT_EXSTS:
			printf("Error copying option to process' table. Alredy an option\n");
		break;

		case ERR_REDIR_FILEXPCTD:
			printf("Error : expected file after IN/OUT redirect operator\n");
		break;

		case ERR_EXEC_FAIL:
			printf("Error swaping process image\n");
		break;

		case ERR_FORK_FAIL:
			printf("Error spawning new child\n");
		break;

		case ERR_SYNTAX_OPERATORS:
			printf("Error : too many operators\n");
		break;

		case ERR_SYNTAX_UNKNOWN:
			printf("Unknows syntax error\n");
		break;

		case ERR_OP_NOT_SUPPORTED:
			printf("Operator not supported!\n");

    default:
    	printf("Error recognizing error : errception\n");
  }
}

//Wrapper functions for system calls
ssize_t read_w(int FD,char *buffer,size_t count){
	ssize_t byteRead = read(FD,buffer,count);
	if(byteRead == -1){
		exit_w(ERR_READ_FAIL);
	}
	return byteRead;
}

ssize_t write_w(int FD, char *buffer,size_t count){
	ssize_t byteWritten = write(FD,buffer,count);
	if(byteWritten == -1){
		perror("Error writing");
		exit_w(ERR_WRITE_FAIL);
	}
	return byteWritten;
}

void lseek_w(int fd,off_t offset,int whence){
	off_t res = lseek(fd,offset,whence);
	if(res < 0){
		printError(ERR_SEEK_FAIL);
		exit(ERR_SEEK_FAIL);
	}
	return;
}

void exit_w(int status){
	printError(status);
	exit(status);
}

void pipe_w(int *pipes){
  int retVal;
  retVal = pipe(pipes);
  if(retVal < 0){
    perror("ERR_PIPE  ");
		exit(ERR_PIPE);
  }
}

int mkstemp_w(char * template){
  int tmpFD = mkstemp(template);
  if(tmpFD < 0){
    perror("ERR_MKSTEMP  ");
    exit(ERR_MKSTEMP);
  }
  return tmpFD;
}

void setenv_w(char * key,char * value){
  int setenv_status;
  setenv_status = setenv(key,value,5);
  if(setenv_status < 0){
    perror("ERR_SETENV  ");
    exit(ERR_SETENV);
  }
}

void setenv_wi(char * key,int value){
  int setenv_status;
  char intAlpha [MAX_CMD_LEN];
  snprintf(intAlpha,sizeof(intAlpha),"%d",value);
  setenv_status = setenv(key,intAlpha,5);
  if(setenv_status < 0){
    perror("ERR_SETENV_I  ");
    exit(ERR_SETENV_I);
  }
}

void waitpid_w(int pid,int *wstatus,int flag){
  int wait_status = waitpid(pid,wstatus,flag);
  if(wait_status < 0){
    perror("ERR_WAIT  ");
    exit(ERR_WAIT);
  }
}

int open_w(char *path,int flag, mode_t mode){
  int fd;
  fd = open(path,flag,mode);
  if(fd < 0){
    perror("ERR_OPEN  ");
    exit(ERR_OPEN);
  }
  return fd;
}

int close_w(int FD){
	int closeResult;
	closeResult = close(FD);
	if(closeResult < 0){
		perror("ERR_CLOSE");
		exit(ERR_CLOSE_FD);
	}
	return closeResult;
}

//Controller's utility functions
char ** getExecArguments(char * executable,processTable_t *table){
		int argvSize = (table -> nOptions) + 3; //Numero di argomenti = numero di opzioni + nome eseguibile + comando + NULL per terminare
		char ** exec_argv = malloc(argvSize*sizeof(char*));
		//Copio nome eseguibile in posizione 0
		exec_argv[0] = malloc(sizeof(char)*MAX_ARG_LEN);
		strcpy(exec_argv[0],executable);
		//Copio comando da eseguire
		exec_argv[1] = malloc(sizeof(char)*MAX_ARG_LEN);
		strcpy(exec_argv[1],table->command);
		//Copio opzioni
		int i;
		for(i=2;i<(argvSize-1);i++){
			exec_argv[i] = malloc(sizeof(char)*MAX_ARG_LEN);
			strcpy(exec_argv[i],table->options[i-2]);
		}
		//Copio NULL in ultima posizione
		exec_argv[argvSize-1] = NULL;

		return exec_argv;
}



void printToken(token_t*token){
	bool VERBOSE = FALSE;
  if(VERBOSE){printf("\nwelcome to printToken\n");}
  switch (token->type) {

    case OPERATOR:
      switch (*((int*)(token -> value))){

        case PIPE:
        	printf("token->value = PIPE\n");
        break;

        case AND:
        	printf("token->value = AND\n");
        break;

        case OR:
        	printf("token->value = OR\n");
        break;

        case IN_REDIRECT:
        	printf("token->value = IN_REDIRECT\n");
        break;

        case OUT_REDIRECT:
        	printf("token->value = OUT_REDIRECT\n");
        break;

        default:
        	printf("Error printing token, no value matched\n");
      }
    break;

    case COMMAND:
      printf("token->type = COMMAND\ntoken->value = %s\n",(char*)token->value);
    break;

    case OPTION:
      printf("token->type = OPTION\ntoken->value = %s\n",(char*)token->value);
    break;

    case REDIR_FILE:
      printf("token->type = FILE\ntoken->value = %s\n",(char*)token->value);
    break;

    default:
    break;
  }
}


bool isOperator(char* inputString,token_t*tokenTmp){
	bool VERBOSE = FALSE;
  if(VERBOSE){printf("\nwelcome to isOperator\n");}
  if(VERBOSE){printf("inputString = %s\n",inputString);}
  bool retVal = FALSE;
    if(strcmp(inputString,"|") == 0){
      if(VERBOSE){printf("matched as PIPE\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = PIPE;
      *((int*)(tokenTmp -> value)) = PIPE;
    }
    else if(strcmp(inputString,"&&") == 0){
      if(VERBOSE){printf("matched as AND\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = AND;
      *((int*)(tokenTmp -> value)) = AND;
    }
    else if(strcmp(inputString,"||") == 0){
      if(VERBOSE){printf("matched as OR\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = OR;
      *((int*)(tokenTmp -> value)) = OR;
    }
    else if(strcmp(inputString,"<") == 0){
      if(VERBOSE){printf("matched as IN_REDIRECT\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = IN_REDIRECT;
      *((int*)(tokenTmp -> value)) = IN_REDIRECT;
    }
    else if(strcmp(inputString,">") == 0){
      if(VERBOSE){printf("matched as OUT_REDIRECT\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = OUT_REDIRECT;
      *((int*)(tokenTmp -> value)) = OUT_REDIRECT;
    }
    else{
      if(VERBOSE){printf("no match found\n");}
    }
  return retVal;
}

token_t **tokenize(char *rawInput,int *tokenNumber){
	bool VERBOSE = FALSE	;
  if(VERBOSE){printf("welcome to tokenize\n\n");}

  token_t ** tokenArray = malloc(MAX_CMD_LEN*sizeof(token_t*));

  int nTokens = 0;
  int charPointer = 0;
  int subStringPointer = 0;
  int wordsCounter = 0;

  char currentChar = rawInput[0];
  char subString[MAX_CMD_LEN];
  char words[MAX_CMD_LEN][MAX_CMD_LEN];

  bool lteq_operator = TRUE;
  bool lteq_command = FALSE;
  bool lteq_option = FALSE;
  bool lteq_redirect = FALSE;
  bool lteq_fileName = FALSE;

  while( (charPointer<MAX_CMD_LEN) && (currentChar != '\0') ){
    currentChar = rawInput[charPointer];

    //Itera fino ad uno spazio, fino a max. dim o EOF
    while( (charPointer<MAX_CMD_LEN) && (currentChar != '\0') && (currentChar != ' ') ){
      if(VERBOSE){printf("currentChar = %c\n",currentChar);}
      subString[subStringPointer] = currentChar;
      subStringPointer += 1;
      charPointer += 1;
      currentChar = rawInput[charPointer];
    }
    //Riconosciuta una stringa
    subString[subStringPointer] = '\0';
    if(VERBOSE){printf("recognized string : ' %s '\n",subString);}
    if(VERBOSE){printf("string length = %ld\n",strlen(subString));}
    strcpy(words[wordsCounter],subString);
    if(VERBOSE){printf("copied ' %s ' in words[%d]\n",words[wordsCounter],wordsCounter);}
    if(VERBOSE){printf("words[wordsCounter] length = %ld\n",strlen(words[wordsCounter]));}

    //Match della stringa riconosciuta
    token_t * tokenTmp = malloc(sizeof(token_t));
    bool isOp = isOperator(words[wordsCounter],tokenTmp);
    //La stringa è un operatore
    if(isOp){
      if(lteq_operator){
        exit_w(ERR_SYNTAX_OPERATORS);
      }
      else if(lteq_redirect){
        exit_w(ERR_REDIR_FILEXPCTD);
      }
      else if(lteq_command || lteq_option || lteq_fileName){
        if( *((int*)(tokenTmp -> value)) == IN_REDIRECT || *((int*)(tokenTmp -> value)) == OUT_REDIRECT){
          if(VERBOSE){printf("matching ' %s ' as redirect operator\n",words[wordsCounter]);}
          tokenArray[nTokens] = tokenTmp;
          nTokens++;
          lteq_operator = FALSE;
          lteq_command = FALSE;
          lteq_option = FALSE;
          lteq_redirect = TRUE;
          lteq_fileName = FALSE;
        }
        else{
          if(VERBOSE){printf("matching ' %s ' as operator\n",words[wordsCounter]);}
          tokenArray[nTokens] = tokenTmp;
          nTokens++;
          lteq_operator = TRUE;
          lteq_command = FALSE;
          lteq_option = FALSE;
          lteq_redirect = FALSE;
          lteq_fileName = FALSE;
        }
      }
    }
    else{
      //Match as fileName for redirect
      if(lteq_redirect){
        if(VERBOSE){printf("matching ' %s ' as file Name\n",words[wordsCounter]);}
        tokenTmp -> type = REDIR_FILE;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordsCounter]);
        tokenArray[(nTokens)] = tokenTmp;
        nTokens++;
        lteq_operator = FALSE;
        lteq_command = FALSE;
        lteq_option = FALSE;
        lteq_redirect = FALSE;
        lteq_fileName = TRUE;
      }
      //Match as command
      else if(lteq_operator){
        if(VERBOSE){printf("matching ' %s ' as command\n",words[wordsCounter]);}
        tokenTmp -> type = COMMAND;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordsCounter]);
        tokenArray[(nTokens)] = tokenTmp;
        nTokens++;
        lteq_operator = FALSE;
        lteq_command = TRUE;
        lteq_option = FALSE;
        lteq_redirect = FALSE;
        lteq_fileName = FALSE;
      }
      //Match as option
      else if(lteq_command || lteq_option){
        if(VERBOSE){printf("matching ' %s ' as an option\n",words[wordsCounter]);}
        tokenTmp -> type = OPTION;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordsCounter]);
        tokenArray[nTokens] = tokenTmp;
        nTokens++;
        lteq_operator = FALSE;
        lteq_command = FALSE;
        lteq_option = TRUE;
        lteq_redirect = FALSE;
        lteq_fileName = FALSE;
      }
      //Error
      else{
				exit_w(ERR_SYNTAX_UNKNOWN);
      }
    }
    if(VERBOSE){printf("\n");}
    wordsCounter += 1;
    strcpy(subString,"");
    subStringPointer = 0;
    charPointer += 1;
  }
  tokenArray[(nTokens)+1] = NULL;
	*tokenNumber = nTokens;
  return tokenArray;
}

void clearTable(processTable_t *table){

	//Pulizia della tabella con valori convenzionalmente privi di significato
  strcpy(table -> command,"");
	int i;
	for(i=0;i<table -> nOptions;i++){
		strcpy(table -> options[i],"");
	}
	table -> nOptions = -1;

	strcpy(table -> tmpOutFile,"");
	strcpy(table -> tmpErrFile,"");
	strcpy(table -> tmpProcInfoFile,"");

	strcpy(table -> outRedirectFile,"");
	strcpy(table -> inputFile,"");

	table -> inputPipe = -1;
	table -> outputPipe = -1;

	table -> pid = -1;
	table -> status = -1;
	table -> skip = FALSE;

	return;
}

void copyTable(processTable_t *tableTo,processTable_t *tableFrom){

	//Copia dei valori di una tabella tableFrom in un altra tabella tableTo
	strcpy(tableTo -> command,tableFrom -> command);

	int i;
	for(i=0;i<tableTo -> nOptions;i++){
		strcpy(tableTo -> options[i],"");
	}
	tableTo -> nOptions = tableFrom -> nOptions;
	for(i=0;i<tableFrom -> nOptions;i++){
		strcpy(tableTo -> options[i],tableFrom -> options[i]);
	}

	strcpy(tableTo -> tmpOutFile,tableFrom -> tmpOutFile);
	strcpy(tableTo -> tmpErrFile,tableFrom -> tmpErrFile);
	strcpy(tableTo -> tmpProcInfoFile,tableFrom -> tmpProcInfoFile);

	strcpy(tableTo -> outRedirectFile,tableFrom -> outRedirectFile);
	strcpy(tableTo -> inputFile,tableFrom -> inputFile);

	tableTo -> inputPipe = tableFrom -> inputPipe;
	tableTo -> outputPipe = tableFrom -> outputPipe;

	tableTo -> pid = tableFrom -> pid;
	tableTo -> status = tableFrom -> status;
	tableTo -> skip = tableFrom -> skip;

	return;
}

void pushToPipesList(pipesList_t **head,pipesList_t **tail,pipesList_t *newElement){
  if(*tail == NULL && *head == NULL){
    newElement->prev = NULL;
    newElement->next = NULL;
    *tail = newElement;
    *head = newElement;
  }
  else{
    (*head)->next = newElement;
    newElement->prev = *head;
    newElement->next = NULL;
    *head = newElement;
  }
  return;
}

void pushToTablesList(processesList_t **head,processesList_t **tail,processesList_t *newElement){
  if(*tail == NULL && *head == NULL){
    newElement->prev = NULL;
    newElement->next = NULL;
    *tail = newElement;
    *head = newElement;
  }
  else{
    (*head)->next = newElement;
    newElement->prev = *head;
    newElement->next = NULL;
    *head = newElement;
  }
  return;
}

void rewindLinkedList(processesList_t **head,processesList_t **tail){
	*tail = *head;
	while( (*tail) -> prev != NULL ){
		(*tail) = (*tail) -> prev;
	}
}

//Print functions
void printTablesList(processesList_t *head,processesList_t *tail){
	printf("\nPRINTING TABLES LIST\n");
  int counter = 0;
  while(tail != NULL){

    printf("\tTable[%d] :\n\t\tcommand = %s\n\t\t",counter,tail->table->command);
		printf("Options :\n");
		int i;
		for(i=0;i<tail->table->nOptions;i++){
			printf("\t\t\toption[%d] = %s\n",i,tail->table->options[i]);
		}
		printf("\t\ttmpOutFile = %s\n",tail -> table -> tmpOutFile);
		printf("\t\ttmpErrFile = %s\n",tail -> table -> tmpErrFile);
		printf("\t\ttmpProcInfoFile = %s\n",tail -> table -> tmpProcInfoFile);

		printf("\t\toutRedirectFile = %s\n",tail -> table -> outRedirectFile);
		printf("\t\tinputRedirectFile = %s\n",tail -> table -> inputFile);

		printf("\t\tinputPipe = %d\n\t\toutputPipe = %d\n",tail -> table -> inputPipe, tail -> table -> outputPipe);
		printf("\t\tpid = %d\n\t\tskip = %d\n\t\tstatus = %d\n",tail -> table -> pid, tail -> table -> skip, tail -> table -> status);

    tail = tail->next;
    counter ++;
  }
	printf("\n");

	return;
}

void printPidsList(pidsList_t *head,pidsList_t *tail){
printf("\nPRINTING PIDS LIST\n");
  int counter = 0;
  while(tail != NULL){
    printf("\tPidsList[%d] :\n\t\tpid = %d\n",counter,tail->pid);
    tail = tail->next;
		counter ++;
  }
	printf("\n");

	return;
}

void printPipesList(pipesList_t *head,pipesList_t *tail){
printf("\nPRINTING PIPES LIST\n");
  int counter = 0;
  while(tail != NULL){
    printf("\tPipeList[%d] :\n\t\tpipe[READ] = %d\n\t\tpipe[WRITE] = %d\n",counter,tail->pipe[READ],tail->pipe[WRITE]);
    tail = tail->next;
		counter ++;
  }
printf("\n");
return;
}
