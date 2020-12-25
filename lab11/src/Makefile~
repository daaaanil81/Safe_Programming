DIR_BIN 		= ../bin

SSL_INCLUDE_PATH 	= -I./../third_party/openssl/include
SSL_LIB_PATH 		= -L./../third_party/openssl/lib

LIBZ_LIB_PATH		= -L./../thirt_party/libz/lib
LIBZ_INCLUDE_PATH	= -I./../thirt_party/libz/include

FLAGS 			= -ggdb -Wall --std=c++11 $(SSL_INCLUDE_PATH) $(LIBZ_INCLUDE_PATH)
LIBS 			= $(SSL_LIB_PATH) $(LIBZ_INCLUDE_PATH) -lcrypto -lssl

BIN_CLIENT		= client
BIN_SERVER		= server
FILES 			= client.o server.o

compile: $(FILES) copy
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN_CLIENT) client.o $(LIBS)
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN_SERVER) server.o $(LIBS)

copy:
	cp client.cpp ../../../test/lab05/src/
	cp server.cpp ../../../test/lab05/src/

client.o: client.cpp
	g++ -c $(FLAGS) client.cpp 

server.o: server.cpp
	g++ -c $(FLAGS) server.cpp 

clean:
	- rm *~
	- rm *.o
	- rm $(DIR_BIN)/$(BIN_CLIENT)
	- rm -rf $(DIR_BIN)/$(BIN_CLIENT).dSYM
	- rm $(DIR_BIN)/$(BIN_SERVER)
	- rm -rf $(DIR_BIN)/$(BIN_SERVER).dSYM
