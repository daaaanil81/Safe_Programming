#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <ctime>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/md5.h"


// Create the SSL socket and intialize the socket address structure
int openListener(int port)
{
    int sd;
    struct sockaddr_in addr;
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        std::cout << "can't bind port" << std::endl;
        abort();
    }
    
    if (listen(sd, 10) != 0 )
    {
        std::cout << "Can't configure listening port" << std::endl;
        abort();
    }
    return sd;
}

static int clientfd = -1;
static int serverfd = -1;
static RSA* rsaClient = nullptr;

void sigint(int sig)
{
    if (clientfd >= 0)
    {        
        shutdown(clientfd, SHUT_RDWR);
        RSA_free(rsaClient);
    }
    if (serverfd >= 0)
    {
        shutdown(serverfd, SHUT_RDWR);
    }

    exit(0);
}

int sendText(RSA* rsa, int sd, const std::string& message, unsigned int size)
{
    unsigned char* responseCrypt = new unsigned char[size];
    const unsigned char* constUnsigedCharMessage = reinterpret_cast<const unsigned char*>(message.c_str());
    int len = RSA_public_encrypt(message.length(), constUnsigedCharMessage, responseCrypt, rsa, RSA_PKCS1_PADDING);
    len = send(sd, responseCrypt, len, 0);
    delete [] responseCrypt;
    return len;
}

int sendText(RSA* rsa, int sd, unsigned char* message, unsigned int length, unsigned int size)
{
    unsigned char* responseCrypt = new unsigned char[size];
    const unsigned char* constUnsigedCharMessage = reinterpret_cast<const unsigned char*>(message);
    int len = RSA_public_encrypt(length, constUnsigedCharMessage, responseCrypt, rsa, RSA_PKCS1_PADDING);
    len = send(sd, responseCrypt, len, 0);
    std::cout << responseCrypt;
    delete [] responseCrypt;
    return len;
}

int recvText(RSA* rsa, int sd, std::string& message, unsigned int size)
{
    unsigned char* responseCrypt = new unsigned char[size];
    message.resize(size);
    const unsigned char* constUnsigedCharMessage = reinterpret_cast<const unsigned char*>(responseCrypt);
    unsigned char* unsignedCharPtr = reinterpret_cast<unsigned char*>(&message[0]);
    recv(sd, responseCrypt, size, 0);
    int len = RSA_private_decrypt(size, constUnsigedCharMessage, unsignedCharPtr, rsa, RSA_PKCS1_PADDING);
    delete [] responseCrypt;
    message.resize(len);
    return len;
}


void recvPassword(int sd) /* Serve the connection -- threadable */
{
    const unsigned long bits = 2048; /* длина ключа в битах */
    const unsigned int sizeBytes = bits / 8;
    std::string requestCrypt(sizeBytes, '\0');
    std::string request(sizeBytes, '\0');

    const char* error = nullptr;
    int len = 0;
    const unsigned char* constUnsigedCharMessage = nullptr;
    unsigned int lenPublicKey = 500;
    unsigned char publicKeyClient[lenPublicKey];
    
    do
    {
        len = recv(sd, publicKeyClient, lenPublicKey, 0);
        error = "Error with recv public key.";
        if (len <= 0)
            break;
        
        lenPublicKey = len;
        
        constUnsigedCharMessage = publicKeyClient;
        rsaClient = d2i_RSAPublicKey(nullptr, &constUnsigedCharMessage, lenPublicKey);
        
        std::ifstream file("../resources/message.txt");
        unsigned char* buffer = nullptr;
        int length = 0;
        file.seekg(0, std::ios::end);    // go to the end
        length = file.tellg();           // report location (this is the length)
        file.seekg(0, std::ios::beg);    // go back to the beginning
        buffer = new unsigned char[length];    // allocate memory for a buffer of appropriate dimension
        file.read(reinterpret_cast<char*>(buffer), length);       // read the whole file into the buffer
        file.close();

        std::cout << buffer << std::endl;
        sendText(rsaClient, sd, std::to_string(length), sizeBytes);
        
        MD5_CTX md5handler;
        unsigned char md5digest[MD5_DIGEST_LENGTH];
        char md5result[MD5_DIGEST_LENGTH * 2];
        char* time = md5result;

        
        MD5_Init(&md5handler);
        for (int i = 0; i < length; i += 16)
        {
            int l = 16;
            if ((length - i) / 16 == 0)
                l = length - i;
            MD5_Update(&md5handler, buffer + i, l);
        }
        MD5_Final(md5digest, &md5handler);

        int n = 0;
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i, time += n)
            n = sprintf(time, "%02x", md5digest[i]);
        
        sendText(rsaClient, sd, md5result, sizeBytes);
        
        for (int i = 0; i < length; i += 16)
        {
            int l = 16;
            if ((length - i) / 16 == 0)
                l = length - i;
            sendText(rsaClient, sd, buffer + i, l, sizeBytes);
        }
        
        delete [] buffer;
        error = nullptr;
    }
    while(0);
    if (error != nullptr)
        std::cout << error << std::endl;
    RSA_free(rsaClient);
    shutdown(sd, SHUT_RDWR);          /* close connection */
}



int main(int argc, char *argv[])
{
    int portnum;

    if ( argc != 2 )
    {
        std::cout << "Usage: " << argv[0] << " <portnum>" << std::endl;
        exit(0);
    }
    
    portnum = std::stoi(argv[1]);
    
    signal(SIGINT, sigint);

    serverfd = openListener(portnum);    /* create server socket */
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        clientfd = accept(serverfd, (struct sockaddr*)&addr, &len);  /* accept connection as usual */
        std::cout << "Connection: " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << std::endl;
        recvPassword(clientfd);
        clientfd = -1;
    }
    shutdown(serverfd, SHUT_RDWR);          /* close server socket */
    return 0;
}
