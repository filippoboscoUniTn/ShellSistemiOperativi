build : setup compile
	@echo build

setup : 
	mkdir bin && cd bin && mkdir tmp

compile : src/logger.c src/controller.c src/shell.c
	gcc -std=gnu90 src/logger.c src/functions.c -o bin/logger
	gcc -std=gnu90 src/controller.c src/functions.c -o bin/controller
	gcc -std=gnu90 src/shell.c src/functions.c -o bin/shell

clean :
	rm -r bin && rm -r tmp
