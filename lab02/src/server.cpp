#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
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

SSL_CTX* initServerCTX(void)
{
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    const SSL_METHOD *method = SSLv23_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void loadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile)
{
    /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        std::cerr << "Private key does not match the public certificate" << std::endl;
        abort();
    }
}

void showCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        std::cout << "Server certificates: " << std::endl;
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        std::cout << "Subject: " << line << std::endl;
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        std::cout << "Issuer: "<< line << std::endl;
        free(line);
        X509_free(cert);
    }
    else
        std::cout << "No certificates." << std::endl;
}

void recvPassword(SSL* ssl) /* Serve the connection -- threadable */
{
    std::string request(40, '\0');
    int sd, bytes;
    do
    {
        if ( SSL_accept(ssl) == -1 )     /* do SSL-protocol accept */
            ERR_print_errors_fp(stderr);
        else
        {
            showCerts(ssl);        /* get any certificates */
            bytes = SSL_read(ssl, &request[0], request.length()); /* get request */
            std::cout << "Client msg: " << request << std::endl;
            if (bytes > 0)
            {
                MD5_CTX md5handler;
                unsigned char md5digest[MD5_DIGEST_LENGTH];
                char md5result[MD5_DIGEST_LENGTH * 2];
                char* time = md5result;
                std::string username;
                std::string password;
                std::string salt = "s5a5l5t";
                int pos = request.find(':');
                username = request.substr(0, pos);

                int pos1 = request.find('\0'); 
                password = request.substr(pos + 1, pos1  - pos - 1);
                std::cout << username << std::endl;
                std::cout << password << std::endl;
                password += salt; 
                MD5_Init(&md5handler);
                MD5_Update(&md5handler, password.c_str(), password.length());
                MD5_Final(md5digest, &md5handler);

                int n = 0;
                for (int i = 0; i < MD5_DIGEST_LENGTH; ++i, time += n)
                    n = sprintf(time, "%02x", md5digest[i]);
                std::cout << md5result << std::endl;
                
                std::ifstream file("../resources/passwords.txt", std::ios_base::in);

                std::string newUsername;
                std::string newHash;
                unsigned char flag = 0;
                while (file >> newUsername >> newHash)
                {
                    
                    std::cout << newUsername << std::endl;
                    if (username == newUsername)
                    {
                        flag = 1;
                        break;
                    }
                }
                file.close();

                if (flag == 0)
                {
                    std::cout << "User wasn't found." << std::endl;
                    SSL_write(ssl, "User wasn't found.", strlen("User wasn't found."));
                    break;
                }
                
                if ( md5result == newHash)
                {
                    std::cout << "Authorization successful." << std::endl;
                    SSL_write(ssl, "Authorization successful.", strlen("Authorization successful."));
                    break;
                }
                std::cout << "Authorization not successful." << std::endl;
                SSL_write(ssl, "Authorization not successful.", strlen("Authorization not successful."));
            }
            else
                ERR_print_errors_fp(stderr);
        }
    }
    while(0);
    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    shutdown(sd, SHUT_RDWR);          /* close connection */
}

int main(int argc, char *argv[])
{
    SSL_CTX *ctx;
    int server;
    int portnum;

    if ( argc != 2 )
    {
        std::cout << "Usage: " << argv[0] << " <portnum>" << std::endl;
        exit(0);
    }
    
    SSL_library_init();
    portnum = std::stoi(argv[1]);

    ctx = initServerCTX();        /* initialize SSL */

    loadCertificates(ctx, "../resources/certificate/localserver.cert", "../resources/certificate/localserver.key"); /* load certs */
    server = openListener(portnum);    /* create server socket */
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;
        int client = accept(server, (struct sockaddr*)&addr, &len);  /* accept connection as usual */
        std::cout << "Connection: " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << std::endl;
        ssl = SSL_new(ctx);              /* get new SSL state with context */
        SSL_set_fd(ssl, client);      /* set connection socket to SSL state */
        recvPassword(ssl);         /* service connection */
    }
    shutdown(server, SHUT_RDWR);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
    return 0;
}
