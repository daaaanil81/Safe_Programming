#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rsa.h"
#include "openssl/md5.h"

int openConnection(const char *hostname, int port)
{
    int sd;
    struct sockaddr_in addr;
    
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, hostname, &addr.sin_addr);
    if (connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        shutdown(sd, SHUT_RDWR);
        std::cout << "Error with connection." << std::endl;
        abort();
    }
    return sd;
}

RSA* generateKeys(unsigned long bits)
{
    BIGNUM* bn = BN_new();
    RSA* rsa = RSA_new();
    BN_set_word(bn, RSA_F4);
/* Генерируем ключи */
    int res = RSA_generate_key_ex(rsa, bits, bn, nullptr);
    if (res == 0)
        exit(0);    
    BN_free(bn);
    return rsa;
}

static int serverfd = -1;
static RSA* rsaClient = nullptr;

void sigint(int sig)
{
    if (serverfd >= 0)
    {
        shutdown(serverfd, SHUT_RDWR);
        RSA_free(rsaClient);
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

int recvText(RSA* rsa, int sd, std::string& message, unsigned int size)
{
    unsigned char* responseCrypt = new unsigned char[size];
    message.resize(size);
    const unsigned char* constUnsigedCharMessage = reinterpret_cast<const unsigned char*>(responseCrypt);
    unsigned char* unsignedCharPtr = reinterpret_cast<unsigned char*>(&message[0]);
    int len = recv(sd, responseCrypt, size, 0);
    if (len <= 0)
    {
        delete [] responseCrypt;
        return len;
    }
    len = RSA_private_decrypt(len, constUnsigedCharMessage, unsignedCharPtr, rsa, RSA_PKCS1_PADDING);
    delete [] responseCrypt;
    message.resize(len);
    return len;
}


int main(int argc, char *argv[])
{
    const unsigned long bits = 2048; /* длина ключа в битах */
    const unsigned int sizeBytes = bits / 8;
    std::string hostname;
    std::string portnum;
    
    if ( argc != 3 )
    {
        std::cout << "usage: " << argv[0] << " <hostname> <portnum>\n" << std::endl;
        return 0;
    }
    hostname = argv[1];
    portnum = argv[2];
    signal(SIGINT, sigint);

    int len = 0;
    const char* error = nullptr;
    unsigned char* unsignedCharPtr = nullptr;
    rsaClient = generateKeys(bits);
    const unsigned int lenPublicKey = i2d_RSAPublicKey(rsaClient, nullptr); 
    const unsigned int lenPrivateKey = i2d_RSAPrivateKey(rsaClient, nullptr);
    unsigned char publicKeyClient[lenPublicKey];
    unsigned char privateKeyClient[lenPrivateKey];
    std::string response(sizeBytes, '\0');
    
    unsignedCharPtr = publicKeyClient;
    i2d_RSAPublicKey(rsaClient, &unsignedCharPtr); 
    std::cout << "lenPublicKey = " << lenPublicKey << std::endl;
    
    unsignedCharPtr = privateKeyClient;
    i2d_RSAPrivateKey(rsaClient, &unsignedCharPtr);
    std::cout << "lenPrivateKey = " << lenPrivateKey << std::endl;

    serverfd = openConnection(hostname.c_str(), std::stoi(portnum));
    do
    {
        if (serverfd >= 0 )
        {
            len = send(serverfd, publicKeyClient, lenPublicKey, 0);
            error = "Error with send public key.";
            if (len != lenPublicKey)
                break;

            error = "Error with recv size message.";
            len = recvText(rsaClient, serverfd, response, sizeBytes);
            if (len <= 0) 
                break;
            int sizeMessage = std::stoi(response);
            std::cout << "Size of message = " << sizeMessage << std::endl;

            error = "Error with recv size message.";
            len = recvText(rsaClient, serverfd, response, sizeBytes);
            if (len <= 0) 
                break;
            
            std::string hash = response;
            std::cout << "Hash = " << hash << std::endl;

            MD5_CTX md5handler;
            unsigned char md5digest[MD5_DIGEST_LENGTH];
            char md5result[MD5_DIGEST_LENGTH * 2];
            char* time = md5result;
            
            response.resize(16);
            std::string message;
            MD5_Init(&md5handler);
            for (int i = 0; i < sizeMessage; i += 16)
            {
                int l = 16;
                if ((sizeMessage - i) / 16 == 0)
                    l = sizeMessage - i;
                recvText(rsaClient, serverfd, response, sizeBytes);
                message += response;
                MD5_Update(&md5handler, response.c_str(), l);
            }
            MD5_Final(md5digest, &md5handler);
            int n = 0;
            for (int i = 0; i < MD5_DIGEST_LENGTH; ++i, time += n)
                n = sprintf(time, "%02x", md5digest[i]);
            
            if (hash == md5result)
                std::cout << message << std::endl;
            else
                std::cout << "Error with hash." << std::endl;
        }
        error = nullptr;
    }
    while(0);
    if (error != nullptr)
        std::cout << error << std::endl;
    shutdown(serverfd, SHUT_RDWR);
    RSA_free(rsaClient);
    return 0;
}
