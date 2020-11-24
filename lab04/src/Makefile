DIR_BIN 		= ../bin

CERTIFICATE 		= ../resources/certificate/lab04.p12  

SSL_INCLUDE_PATH 	= -I./../third_party/openssl/include
SSL_LIB_PATH 		= -L./../third_party/openssl/lib

QUICKMAIL_INCLUDE_PATH 	= -I./../third_party/libquickmail/include
QUICKMAIL_LIB_PATH 	= -L./../third_party/libquickmail/lib

CURL_INCLUDE_PATH 	= -I./../third_party/curl/include
CURL_LIB_PATH 		= -L./../third_party/curl/lib

SSH2_INCLUDE_PATH 	= -I./../third_party/libssh2/include
SSH2_LIB_PATH 		= -L./../third_party/libssh2/lib

LIBZ_LIB_PATH		= -L./../thirt_party/libz/lib
LIBZ_INCLUDE_PATH	= -I./../thirt_party/libz/include

FRAMEWORK 		= -framework Security

FLAGS 			= -ggdb -Wall --std=c++11 $(QUICKMAIL_INCLUDE_PATH) $(CURL_INCLUDE_PATH) $(SSH2_INCLUDE_PATH) $(SSL_INCLUDE_PATH) $(LIBZ_INCLUDE_PATH)
LIBS 			= $(QUICKMAIL_LIB_PATH) $(CURL_LIB_PATH) $(SSH2_LIB_PATH) $(SSL_LIB_PATH) $(LIBZ_INCLUDE_PATH) -lcrypto -lssl -lssh2 -lz -lquickmail -lquickmaillight -lcurl -lssl -lcrypto -lcurl -lssh2 -lldap -lz -lcrypto -lssl  -lcurl $(FRAMEWORK) 
BIN_CLIENT		= client
BIN_SERVER		= server
FILES 			= client.o server.o

compile: $(FILES)
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN_CLIENT) client.o $(LIBS)
	g++ $(FLAGS) -o $(DIR_BIN)/$(BIN_SERVER) server.o $(LIBS)
	sudo security import $(CERTIFICATE)
	codesign -s lab04 $(DIR_BIN)/$(BIN_CLIENT)
	codesign -s lab04 $(DIR_BIN)/$(BIN_SERVER)
	codesign -dv --verbose=4 $(DIR_BIN)/$(BIN_CLIENT)
	codesign -dv --verbose=4 $(DIR_BIN)/$(BIN_SERVER)	
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
