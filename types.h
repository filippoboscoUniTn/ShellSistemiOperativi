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






#endif
