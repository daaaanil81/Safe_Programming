#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

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

SSL_CTX* initCTX(void)
{
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    const SSL_METHOD *method = SSLv23_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void showCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        std::cout << "Server certificates:" << std::endl;
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        std::cout << "Subject: " << line << std::endl;
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        std::cout << "Issuer: " << line << std::endl;
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        std::cout << "Info: No client certificates configured." << std::endl;
}

static int serverStatic;
static SSL *sslStatic = nullptr;
static SSL_CTX* ctxStatic = nullptr;

void sigint(int sig)
{
    if (sslStatic != nullptr)
    {
        SSL_free(sslStatic);
        shutdown(serverStatic, SHUT_RDWR);
        SSL_CTX_free(ctxStatic); 
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    //SSL_CTX *ctx;
    //int server;
    //SSL *ssl;
    int bytes;
    std::string hostname;
    std::string portnum;
    
    if ( argc != 3 )
    {
        std::cout << "usage: " << argv[0] << " <hostname> <portnum>\n" << std::endl;
        return 0;
    }

    SSL_library_init();

    hostname = argv[1];
    portnum = argv[2];
    signal(SIGINT, sigint);
    
    serverStatic = openConnection(hostname.c_str(), std::stoi(portnum));

    ctxStatic = initCTX();
    sslStatic = SSL_new(ctxStatic);      /* create new SSL connection state */
    SSL_set_fd(sslStatic, serverStatic);    /* attach the socket descriptor */
    
    if ( SSL_connect(sslStatic) == -1 )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {
        std::string username;
        std::string password;
        std::string message;
        std::string response(1024, '\0');
        std::cout << "Enter the User Name : " << std::endl;
        std::cin >> username;

        std::cout << "Enter the Password : " << std::endl;
        std::cin >> password;
        
        message = username + ":" + password;  

        std::cout << "Connected with " << SSL_get_cipher(sslStatic) << " encryption" << std::endl;
        
        showCerts(sslStatic);        /* get any certs */
        SSL_write(sslStatic, message.c_str(), message.length());   /* encrypt & send message */
        
        bytes = SSL_read(sslStatic, &response[0], response.length()); /* get reply & decrypt */
        std::string str = "Password was send in your email.";
        response.resize(bytes);
        
        if (response == str)
        {
            response.resize(1024, '\0');
            std::string timePass;
            std::cin >> timePass;
            SSL_write(sslStatic, timePass.c_str(), timePass.length());
            bytes = SSL_read(sslStatic, &response[0], response.length()); /* get reply & decrypt */
        }
        
        std::cout << "Received: " << response << std::endl;
        SSL_free(sslStatic);        /* release connection state */
    }
    shutdown(serverStatic, SHUT_RDWR);         /* close socket */
    SSL_CTX_free(ctxStatic);        /* release context */
    return 0;
}
