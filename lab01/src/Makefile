DIR_BIN 		= ../bin
DIR_LIB_PNG		= -L./../thirt_party/libpng/lib
DIR_INCLUDE_PNG		= -I./../thirt_party/libpng/include
DIR_LIB_Z		= -L./../thirt_party/libz/lib
DIR_INCLUDE_Z		= -I./../thirt_party/libz/include
FLAGS 			= -ggdb -Wall --std=c++11 $(DIR_INCLUDE_PNG) $(DIR_INCLUDE_Z)
LIBS 			= $(DIR_LIB_PNG) $(DIR_LIB_Z) -lpng16 -lz
BIN			= main
FILES 			= main.o

compile: $(FILES)
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN) $(FILES) $(LIBS)
	rm $(FILES)

main.o: main.cpp
	g++ -c $(FLAGS) main.cpp 

clean:
	- rm *~
	- rm *.o
	- rm $(DIR_BIN)/$(BIN)
	- rm -rf $(DIR_BIN)/$(BIN).dSYM
