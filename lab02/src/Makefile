DIR_BIN 		= ../bin
SSL_INCLUDE_PATH 	= -I./../third_party/openssl/include
SSL_LIB_PATH 		= -L./../third_party/openssl/lib
FLAGS 			= -ggdb -Wall --std=c++11 $(SSL_INCLUDE_PATH)
LIBS 			= $(SSL_LIB_PATH) -lcrypto -lssl
BIN_CLIENT		= client
BIN_SERVER		= server
FILES 			= client.o server.o

compile: $(FILES)
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN_CLIENT) client.o $(LIBS)
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN_SERVER) server.o $(LIBS)
#	rm $(FILES)

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
