#include "std_libraries.h"
#include "macros.h"
#include "types.h"
#include "functions.h"

bool VERBOSE = FALSE;

void printToken(token_t*token){
  if(VERBOSE){printf("\nwelcome to printToken\n");}
  switch (token->type) {

    case OPERATOR:
      printf("token->type = OPERATOR\n");
      switch ((long)token->value){

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

    case FILE:
      printf("token->type = FILE\ntoken->value = %s\n",(char*)token->value);
    break;

    default:
    break;
  }
}

bool isOperator(char* inputString,token_t*tokenTmp){
  if(VERBOSE){printf("\nwelcome to isOperator\n");}
  if(VERBOSE){printf("inputString = %s\n",inputString);}
  bool retVal = FALSE;
    if(strcmp(inputString,"|") == 0){
      if(VERBOSE){printf("matched as PIPE\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = PIPE;
      tokenTmp -> value = PIPE;
    }
    else if(strcmp(inputString,"&&") == 0){
      if(VERBOSE){printf("matched as AND\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = AND;
      tokenTmp -> value = AND;
    }
    else if(strcmp(inputString,"||") == 0){
      if(VERBOSE){printf("matched as OR\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = OR;
      tokenTmp -> value = OR;
    }
    else if(strcmp(inputString,"<") == 0){
      if(VERBOSE){printf("matched as IN_REDIRECT\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = IN_REDIRECT;
      tokenTmp -> value = IN_REDIRECT;
    }
    else if(strcmp(inputString,">") == 0){
      if(VERBOSE){printf("matched as OUT_REDIRECT\n");}
      retVal = TRUE;
      tokenTmp -> type = OPERATOR;
      tokenTmp -> value = malloc(sizeof(int));
      //*(int*)(tokenTmp->value) = OUT_REDIRECT;
      tokenTmp -> value = OUT_REDIRECT;
    }
    else{
      if(VERBOSE){printf("no match found\n");}
    }
  return retVal;
}


token_t **tokenize(char *rawInput){
  if(VERBOSE){printf("welcome to tokenize\n\n");}

  token_t * tokenArray[MAX_CMD_LEN];

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
        printf("Error : 2 consecutive operators found!\n");
        //exit(EXIT_FAILURE);
      }
      else if(lteq_redirect){
        printf("Error : expected file after redirect operator, found operator\n");
        //exit(EXIT_FAILURE);
      }
      else if(lteq_command || lteq_option || lteq_fileName){
        if( (long)tokenTmp -> value == IN_REDIRECT || (long)tokenTmp -> value == OUT_REDIRECT){
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
        tokenTmp -> type = FILE;
        tokenTmp -> value = malloc(MAX_FILE_NAME_LEN*sizeof(char));
        strcpy((tokenTmp -> value),words[wordsCounter]);
        tokenArray[nTokens] = tokenTmp;
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
        tokenArray[nTokens] = tokenTmp;
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
        printf("Error : unknown syntx error\n");
      }
    }
    if(VERBOSE){printf("\n");}
    wordsCounter += 1;
    strcpy(subString,"");
    subStringPointer = 0;
    charPointer += 1;
  }
  int i;
  for(i=0;i<nTokens;i++){
    printToken(tokenArray[i]);
    printf("\n");
  }

}


int main(int argc,char **argv){
  if(argc > 2){
    if(strcmp(argv[2],"--v") == 0 ||strcmp(argv[2],"-v") == 0){
      VERBOSE = TRUE;
    }
  }
  char rawInput[MAX_CMD_LEN];
  strcpy(rawInput,argv[1]);
  token_t **inputTokensTmp = tokenize(rawInput);

  return 0;
}
