#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <signal.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include "openssl/md5.h"
#define SIZE 20480

// Create the SSL socket and intialize the socket address structure
int openListener(int port)
{
    int sd;
    struct sockaddr_in addr;
    int enable = 1;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {        
        perror("bind");
        exit(1);
    }
    
    if (listen(sd, 10) != 0 )
    {
        perror("listen");
        exit(1);
    }
    return sd;
}

void sigint(int sig)
{
    std::cout << "Sign Int" << std::endl;
}

int recvRequest(int sd, unsigned char* merkleRoot, const std::string& nameFile, std::vector<std::vector<int>>& ports, int& portInt) /* Serve the connection -- threadable */
{
    int count = ports.size();
    int index = -1;
    do
    {
        std::string request(128, '\0');
        unsigned char response[256] = {0};
        int len = recv(sd, &request[0], request.length(), 0);
        if (len <= 0)
        {
            std::cout << "Error with recv." << std::endl;
            break;
        }
        request.resize(len);

        int s = request.find('/');
        if (s == std::string::npos)
        {
            std::cout << "Error with command." << std::endl;
            break;
        }
        std::cout << request << std::endl;
        std::string command = request.substr(0, s);
        if (command == "File")
        {
            std::string nameFileFromClient = request.substr(s + 1);
            if (nameFile != nameFileFromClient)
            {
                send(sd, "This file wasn't found", sizeof("This file wasn't found"), 0);
                std::cout << "This file wasn't found" << std::endl;
                break;
            }
            std::string text = "File/";
            text += std::to_string(count);
            text += "/";
            memcpy(response, text.c_str(), text.length());
            memcpy(response + text.length(), merkleRoot, MD5_DIGEST_LENGTH);
            len = send(sd, response, text.length() + MD5_DIGEST_LENGTH, 0);
            if (len < text.length() + MD5_DIGEST_LENGTH)
            {
                std::cout << "Error with send." << std::endl;
                break;
            }
        }
        else if(command == "Port")
        {
            unsigned char merkleRootClient[MD5_DIGEST_LENGTH] = {0};
            int p = request.find('/', s + 1);
            std::string indexBlockString = request.substr(s + 1, p - s - 1);
            int indexBlock = std::stoi(indexBlockString);
            memcpy(merkleRootClient, &request[p + 1], MD5_DIGEST_LENGTH);
            if (memcmp(merkleRootClient, merkleRoot, MD5_DIGEST_LENGTH) != 0)
            {
                std::cout << "This file wasn't found" << std::endl;
                send(sd, "This file wasn't found", sizeof("This file wasn't found"), 0);
                break;
            }
            if (indexBlock < 0 && indexBlock >= count)
            {
                std::cout << "Error with index from client" << std::endl;
                send(sd, "Error with index from client", sizeof("Error with index from client"), 0);
                break;
            }
            
            if (ports[indexBlock].size() == 0)
            {
                std::cout << "Empty port" << std::endl;
                send(sd, "Empty port", sizeof("Empty port"), 0);
                break;
            }
            
            int i = rand() % ports[indexBlock].size();
            int port = ports[indexBlock][i];
            std::cout << "Port for client = " << port << std::endl;

            std::string text = "Port/";
            text += std::to_string(port);
            memcpy(response, text.c_str(), text.length());
            len = send(sd, response, text.length(), 0);
            std::cout << response << std::endl;
            if (len < text.length())
            {
                std::cout << "Error with send." << std::endl;
                break;
            }
            
        }
        else if(command == "Set")
        {
            unsigned char merkleRootClient[MD5_DIGEST_LENGTH] = {0};
            int p = request.find('/', s + 1);
            std::string indexHashsString= request.substr(s + 1, p - s - 1);
            s = request.find('/', p + 1);
            std::string portString = request.substr(p + 1, s - p - 1);
            portInt = std::stoi(portString);

            memcpy(merkleRootClient, &request[s + 1], MD5_DIGEST_LENGTH);

            if (memcmp(merkleRootClient, merkleRoot, MD5_DIGEST_LENGTH) != 0)
            {
                std::cout << "This file wasn't found" << std::endl;
                break;
            }
            if (indexHashsString == "All")
                return count;
            else
                return std::stoi(indexHashsString);
        }
    }
    while(0);
    return index;
}

int readDatabase(std::vector<unsigned char*>& vectorStringHashs, const std::string& nameFile)
{
    std::string name = "../resources/SettingsServerHash";
    name += nameFile.substr(0, nameFile.length() - 3);
    name += "txt";
    std::ifstream file(name);
    int index = 0;
    int size = 0;
    int count = 0;
    int length = 0;
    file.seekg(0, std::ios::end);    // go to the end
    length = file.tellg();           // report location (this is the length)
    if(length < 0)
    {
        std::cout << "File with settings Empty." << std::endl;
        file.close();
        return count;
    }
    file.seekg(0, std::ios::beg);    // go back to the beginning
    file >> count;
    file >> size;
    vectorStringHashs.resize(size, nullptr);
    
    for(int i = 0; i < vectorStringHashs.size(); ++i)
    {
        vectorStringHashs[i] = new unsigned char[MD5_DIGEST_LENGTH];
        file >> index;
        for(int k = 0; k < MD5_DIGEST_LENGTH; ++k)
        {
            std::string time;
            file >> time;
            
            if (time == "-1")
            {
                delete [] vectorStringHashs[i];
                vectorStringHashs[i] = nullptr;
                break;
            }
            vectorStringHashs[i][k] = std::stoi(time, nullptr, 16);
        }
        
    }
    
    file.close();
    return count;
}

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(2) 
         << std::hex << i;
  return stream.str();
}

void writeDatabase(std::vector<unsigned char*>& vectorStringHashs, const std::string& nameFile, int count)
{
    std::string name = "../resources/SettingsServerHash";
    name += nameFile.substr(0, nameFile.length() - 3);
    name += "txt";
    if (vectorStringHashs.size() == 0)
        return;
    std::ofstream file(name);
    file << count << '\n';
    file << vectorStringHashs.size() << '\n';
    for(int i = 0; i < vectorStringHashs.size(); ++i)
    {
        file << i;
        file << " ";
        if (vectorStringHashs[i] == nullptr)
        {
            file << "-1" << "\n";
            continue;
        }
        for(int k = 0; k < MD5_DIGEST_LENGTH; ++k)
            file << int_to_hex((unsigned int)vectorStringHashs[i][k]) << " ";
        file << "\n";
    }
    
    for (int i = 0; i < count; ++i)
    {
        if (vectorStringHashs[i] != nullptr)
            delete [] vectorStringHashs[i];
    }
    file.close();
}


int main(int argc, char *argv[])
{
    int portnum;
    if ( argc != 3 )
    {
        std::cout << "Usage: " << argv[0] << " <portnum> <fullPathFile>" << std::endl;
        exit(0);
    }
    srand(time(nullptr));
    std::string fullPath = argv[2];
    int p = fullPath.find_last_of('/');
    std::string nameFile = fullPath.substr(p + 1);
    int clientfd = -1;
    int serverfd = -1;
    int count = 0;
    int sizeBlock = SIZE;
    std::vector<unsigned char*> nodeHashs;
    portnum = std::stoi(argv[1]);
    std::cout << "Open" << std::endl;
    serverfd = openListener(portnum);
    count = readDatabase(nodeHashs, nameFile);
    if (count == 0)
    {
        std::ifstream file(fullPath);
        unsigned char* buffer = nullptr;
        unsigned int length = 0;
        file.seekg(0, std::ios::end);    // go to the end
        length = file.tellg();           // report location (this is the length)
        file.seekg(0, std::ios::beg);    // go back to the beginning
        std::cout << fullPath << std::endl;
        buffer = new unsigned char[length];    // allocate memory for a buffer of appropriate dimension
        std::cout << length << std::endl;
    
        file.read(reinterpret_cast<char*>(buffer), length);       // read the whole file into the buffer
        file.close();
        count = length / sizeBlock;
        if (length % sizeBlock != 0)
            ++count;
        int countBlocks = 1;
        while (countBlocks < count)
            countBlocks *= 2;
        int countHashs = 0; // ====> 31
        int time = countBlocks;
        while (time != 0)
        {
            countHashs += time;
            time = time / 2;
        }
        nodeHashs.resize(countHashs, nullptr);
        int index = countBlocks - 1; // 15 
        for (int j = 0; j < count; ++j)
        {
            MD5_CTX md5handler;
            unsigned char* md5digest = new unsigned char[MD5_DIGEST_LENGTH];
            if ((j + 1) == count && length % sizeBlock != 0)
                sizeBlock = length % sizeBlock;
            MD5_Init(&md5handler);
            for (int i = 0; i < sizeBlock; i += MD5_DIGEST_LENGTH)
            {
                int l = MD5_DIGEST_LENGTH;
                if ((sizeBlock - i) / MD5_DIGEST_LENGTH == 0)
                    l = sizeBlock - i;
                MD5_Update(&md5handler, buffer + i + j * SIZE, l);
            }
            MD5_Final(md5digest, &md5handler);
            nodeHashs[index++] = md5digest;
        }
    
        unsigned char* ptr = nodeHashs[index - 1];
        while (index != nodeHashs.size())
            nodeHashs[index++] = ptr;

        index = countBlocks - 1; // 15, 7
        time = countHashs; // 31, 15
        std::cout << time << " " << index << std::endl;            
        while (index != 0) // 15, 7
        {
            for (int k = index; k < time; k += 2)
            {
                MD5_CTX md5handler;
                MD5_Init(&md5handler);
                unsigned char* md5digest = new unsigned char[MD5_DIGEST_LENGTH];
                MD5_Update(&md5handler, nodeHashs[k], MD5_DIGEST_LENGTH);
                MD5_Update(&md5handler, nodeHashs[k + 1], MD5_DIGEST_LENGTH);
                MD5_Final(md5digest, &md5handler);
                nodeHashs[(k - 1) / 2] = md5digest;
            }
            time = time / 2; // 15, 7, 3, 1
            index = index / 2; // 7, 3, 1, 0
        }
    
        delete [] buffer;
    }
    
    fd_set active_fd_set, read_fd_set;
    timeval timeout = {2,500000}; 
    FD_ZERO(&active_fd_set);
    FD_SET(serverfd, &active_fd_set);
    signal(SIGINT, sigint);
    std::vector<std::vector<int>> ports(count);
    while (true)
    {
        read_fd_set = active_fd_set;
        if (select (serverfd + 1, &read_fd_set, NULL, NULL, &timeout) < 0)
        {
            perror ("select");
            break;
        }
        if (FD_ISSET(serverfd, &read_fd_set))
        {
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            clientfd = accept(serverfd, (struct sockaddr*)&addr, &len); 
            if (clientfd < 0)
            {
                perror("accept");
                break;
            }
            std::cout << "Connection: " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << std::endl;
            int portInt = 0;
            int indexBlock = recvRequest(clientfd, nodeHashs[0], nameFile, ports, portInt);
            if (indexBlock == count)
            {
                for (int i = 0; i < count; ++i)
                    ports[i].push_back(portInt);
            }
            else if (indexBlock != -1 && portInt != 0)
            {
                ports[indexBlock].push_back(portInt);
            }
            shutdown(clientfd, SHUT_RDWR);
            close(clientfd);
            clientfd = -1;
        }
    }
    shutdown(serverfd, SHUT_RDWR);
    writeDatabase(nodeHashs, nameFile, count);
    return 0;
}
