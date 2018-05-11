#commento

execOSX : shell.o taskController
	gcc shell.o -o shell

shell.o : shell.c
	gcc -c shell.c -o shell.o

taskController : taskController.c
	gcc -c taskController.c -o taskController.o
	gcc taskController.o -o taskController
