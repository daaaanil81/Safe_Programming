DIR_BIN 		= ../bin

FLAGS 			= -ggdb -Wall
BIN			= lab11
FILES 			= lab11.o

compile: $(FILES)
	gcc $(FLAGS) -o $(DIR_BIN)/$(BIN) lab11.o 

lab11.o: lab11.c
	gcc -c $(FLAGS) lab11.c 

clean:
	- rm *~
	- rm *.o
	- rm $(DIR_BIN)/$(BIN)
	- rm -rf $(DIR_BIN)/$(BIN).dSYM
