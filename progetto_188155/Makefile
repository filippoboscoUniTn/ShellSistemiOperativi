#cleans all the generated code keeping only source files
clean:
	@echo "Cleaning all generated sources..."
	@rm -r -f bin
	@echo "Cleaned."


#creates:
#bin directory for storing executables
#tmp directory for storing temporary files
setup: clean
	@echo "Setting up directories..."
	@mkdir bin && cd bin && mkdir tmp && mkdir logs
	@echo "Done."


#---------- unit compilation rules in case of changes to one single source file ----------
#for the compilation of the logger we need all the libraries and functions used
compile_logger: src/libs/functions.h src/libs/functions.c src/libs/macros.h src/libs/std_libraries.h src/libs/types.h src/logger.c
	@echo "Compiling logger..."
	gcc -std=gnu90 src/logger.c src/libs/functions.c -o bin/logger
	@echo "Finished."

#for compiling the controller we need also the logger executable
compile_controller: src/libs/functions.h src/libs/functions.c src/libs/macros.h src/libs/std_libraries.h src/libs/types.h src/controller.c bin/logger
	@echo "Compiling controller..."
	gcc -std=gnu90 src/controller.c src/libs/functions.c -o bin/controller
	@echo "Finished."

#for compiling the shell we need the logger executable
compile_shell: src/libs/functions.h src/libs/functions.c src/libs/macros.h src/libs/std_libraries.h src/libs/types.h src/shell.c bin/controller
	@echo "Compiling shell..."
	gcc -std=gnu90 src/shell.c src/libs/functions.c -o bin/shell
	@echo "Finished."

compile_shell_g: src/libs/functions.h src/libs/functions.c src/libs/macros.h src/libs/std_libraries.h src/libs/types.h src/shell.c bin/controller
		@echo "Compiling shell..."
		gcc -g -std=gnu90 src/shell.c src/libs/functions.c -o bin/shell
		@echo "Finished."
#-----------------------------------------------------------------------------------------


#entire compilation rule
compile: src/libs/functions.h src/libs/functions.c src/libs/macros.h src/libs/std_libraries.h src/libs/types.h src/logger.c src/controller.c src/shell.c
	@echo "Compiling entire shell..."
	gcc -std=gnu90 src/logger.c src/libs/functions.c -o bin/logger
	gcc -std=gnu90 src/controller.c src/libs/functions.c -o bin/controller
	gcc -std=gnu90 src/shell.c src/libs/functions.c -o bin/shell
	@echo "Finished."

#build rule
#one rule for doing all
build: setup compile
	@echo "Builded."

rund:
	cd bin && ./shell -out logs/outfile.txt

cleand: bin/tmp
	@rm -r -f bin/tmp
	@echo "Cleaned for debug"

clean_logs: bin/logs
	@rm -r -f bin/logs
	@echo "Logs cleaned."
