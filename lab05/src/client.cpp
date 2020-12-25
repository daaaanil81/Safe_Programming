#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <resolv.h>
#include <sys/uio.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rsa.h"
#include "openssl/md5.h"
#include <mutex>
#define SIZE 20480 
static std::mutex g_lock;


static int clientfd = -1;
static int serverfd = -1;
static int cfd = -1;
static bool flag = true;

void sigint(int sig)
{
    g_lock.lock();
    if (clientfd >= 0)
        shutdown(clientfd, SHUT_RDWR);

    if (serverfd >= 0)
        shutdown(serverfd, SHUT_RDWR);

    if (cfd >= 0)
        shutdown(cfd, SHUT_RDWR);
    g_lock.unlock();
    flag = false;
    std::cout << "Sigint" << std::endl;
}

void recvRequest(std::vector<unsigned char*>& nodeHashs, std::vector<std::string>& fileInBlocks, int count, int countHashs)
{
    fd_set active_fd_set, read_fd_set;
    timeval timeout = {2,500000};

    g_lock.lock();
    unsigned char* merkleRoot = nodeHashs[0];
    g_lock.unlock();

    FD_ZERO(&active_fd_set);
    FD_SET(serverfd, &active_fd_set);
    while (flag)
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
            cfd = accept(serverfd, (struct sockaddr*)&addr, &len); 
            if (cfd < 0)
            {
                perror("accept");
                break;
            }

            g_lock.lock();
            do
            {
                std::string request(128, '\0');
                int len = recv(cfd, &request[0], request.length(), 0);
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

                std::string command = request.substr(0, s);
                if (command == "Get")
                {
                    unsigned char merkleRootClient[MD5_DIGEST_LENGTH] = {0};
                    int p = request.find('/', s + 1);
                    std::string indexString = request.substr(s + 1, p - s - 1);
                    int indexInt = std::stoi(indexString);
                    memcpy(merkleRootClient, &request[p + 1], MD5_DIGEST_LENGTH);
                    if (memcmp(merkleRootClient, merkleRoot, MD5_DIGEST_LENGTH) != 0)
                    {
                        std::cout << "This file wasn't found" << std::endl;
                        send(cfd, "Get error", sizeof("Get error"), 0);
                        break;
                    }
                    if (indexInt >=0 && indexInt < count && fileInBlocks[indexInt].size() != 0)
                    {
                        std::cout << "Send blocks" << std::endl;
                        send(cfd, fileInBlocks[indexInt].c_str(), fileInBlocks[indexInt].length(), 0);
                        break;
                    }
                    std::cout << "Get error" << std::endl;
                    send(cfd, "Get error", sizeof("Get error"), 0);
                    break;
                }
                else if(command == "Val")
                {
                    unsigned char merkleRootClient[MD5_DIGEST_LENGTH] = {0};
                    unsigned char blockHash[MD5_DIGEST_LENGTH] = {0};
                    
                    memcpy(merkleRootClient, &request[++s], MD5_DIGEST_LENGTH);
                    s += MD5_DIGEST_LENGTH;  
                    memcpy(blockHash, &request[++s], MD5_DIGEST_LENGTH);
                    if (memcmp(merkleRootClient, merkleRoot, MD5_DIGEST_LENGTH) != 0)
                    {
                        std::cout << "This file wasn't found" << std::endl;
                        send(cfd, "Val error", sizeof("Val error"), 0);
                        break;
                    }
                    bool f = false;
                    int index = 0;
                    std::cout << "###################################" << std::endl;
                    for (int i = 0; i < count; ++i)
                    {
                        index = i + count - 1;
                        std::cout << "INDEX = " << index << std::endl;
                        if (memcmp(nodeHashs[i + count - 1], blockHash, MD5_DIGEST_LENGTH) == 0)
                        {
                            f = true;
                            
                            break;
                        }
                    }
                    if (!f)
                    {
                        std::cout << "This file wasn't found" << std::endl;
                        send(cfd, "Val error", sizeof("Val error"), 0);
                        break;
                    }
                    int F = index;
                    int S = 0;
                    std::vector<int> indexs;
                    
                    while (F != 0)
                    {
                        std::cout << "F = " << F << std::endl;
                        if (F % 2 == 0)
                            S = F - 1;
                        else
                            S = F + 1;
                        indexs.push_back(S);
                        std::cout << "S = " << S << std::endl;
                        
                        F = (F - 1) / 2;
                    }

                    
                    const int s = indexs.size();
                    struct msghdr msg;
                    struct iovec io[s];

                    msg.msg_name = nullptr;

                    
                    for (int i = 0; i < indexs.size(); ++i)
                    {
                        io[i].iov_base = nodeHashs[indexs[i]];
                        io[i].iov_len = MD5_DIGEST_LENGTH;
                    }
                    msg.msg_iov = io;
                    msg.msg_iovlen = s;
                    sendmsg(cfd, &msg, 0);
                }
            }
            while(0);

            
            g_lock.unlock();
            shutdown(cfd, SHUT_RDWR);
            close(cfd);
            cfd = -1;
        }
        std::cout << "Check" << std::endl;
    }
}
 
int sendCommand(const std::string& command, unsigned char* merkleRoot, int index, int fd)
{
    int sizeText = command.length();
    std::string request(256, '\0');
    std::string indexString;
    memcpy(&request[0], command.c_str(), sizeText);

    indexString = std::to_string(index);
    memcpy(&request[sizeText], indexString.c_str(), indexString.length());
    sizeText += indexString.length();
    request[sizeText++] = '/'; 
    
    memcpy(&request[sizeText], merkleRoot, MD5_DIGEST_LENGTH);
    
    sizeText += MD5_DIGEST_LENGTH;
    request.resize(sizeText);
    std::cout << request << std::endl;
    return send(fd, request.c_str(), request.length(), 0);
}

int openConnection(const char *hostname, int port, int& fd)
{
    struct sockaddr_in addr;
    if (fd > 0)
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, hostname, &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(fd);
        perror("connect");
    }
    return fd;
}

int openListener(int& port)
{
    int sd;
    struct sockaddr_in addr;
    int enable = 1;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    std::cout << "########### Port ############\n" << port << std::endl;
    addr.sin_addr.s_addr = INADDR_ANY;
    do
    {
        port += 1;
        
        addr.sin_port = htons(port);
        if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) == 0 )
        {
            std::cout << "Was opened port = " << port << std::endl;
            break;
        }
    }
    while(port <= 65000);
    
    if (listen(sd, 1) != 0 )
    {
        perror("listen");
        exit(1);
    }
    return sd;
}



int readDatabase(std::vector<unsigned char*>& vectorStringHashs, const std::string& nameFile)
{
    std::string name = "../resources/SettingsClientHash";
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
    std::string name = "../resources/SettingsClientHash";
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

int readFile(std::vector<std::string>& fileInBlocks, const std::string& pathFileName)
{
    std::ifstream file(pathFileName);
    int length = 0;
    int sizeBlock = SIZE;
    int size = 0;
    file.seekg(0, std::ios::end);    // go to the end
    length = file.tellg();           // report location (this is the length)
    file.seekg(0, std::ios::beg);    // go back to the beginning
    
    while (length != 0)
    {
        fileInBlocks[size].resize(sizeBlock, '\0');
        file.read(&fileInBlocks[size][0], sizeBlock);       // read the whole file into the buffer
        int rb = file.gcount();
        std::cout << "Size read from file = " << file.gcount() << std::endl;
        fileInBlocks[size].resize(rb);
        length -= rb;
        ++size;
    }
    file.close();
    return size;
}

void writeFile(const std::vector<std::string>& fileInBlocks, const std::string& pathFileName, int size)
{
    std::ofstream file(pathFileName);
    for (int i = 0; i < size; ++i)
        file << fileInBlocks[i];
    file.close();
}

int sendPort(unsigned char* merkleRoot, int index, int& fd)
{
    return sendCommand("Port/", merkleRoot, index, fd);
}
                
int recvPort(int fd)
{
    std::string response(256, '\0');
                    
    int len = recv(fd, &response[0], response.length(), 0);
    if (len <= 0)
    {
        std::cout << "Error with recv size message." << std::endl;
        return 0;
    }
    response.resize(len);
    std::cout << response << std::endl;
    if (response == "Empty port")
        return -1; 

    int s = response.find('/');
    if (s == std::string::npos)
        return -1;

    std::string portClientString = response.substr(s + 1);
    int portClientInt = std::stoi(portClientString);
    return portClientInt;
}

int sendSet(unsigned char* merkleRoot, const std::string& text, int portServer, int& fd)
{
    std::string command = "Set/";
    command += text;
    return sendCommand(command, merkleRoot, portServer, fd);
}

int sendSet(unsigned char* merkleRoot, int index, int portServer, int& fd)
{
    std::string command = "Set/";
    std::string indexString = std::to_string(index);
    command += indexString;
    command += "/";
    return sendCommand(command, merkleRoot, portServer, fd);    
}

int sendGetBlock(unsigned char* merkleRoot, int index, int fd)
{
    return sendCommand("Get/", merkleRoot, index, fd);
}

int sendVal(const std::string& command, unsigned char* merkleRoot, unsigned char* block, int fd)
{
    int sizeText = command.length();
    std::string request(256, '\0');
    std::string indexString;
    memcpy(&request[0], command.c_str(), sizeText);

    memcpy(&request[sizeText], merkleRoot, MD5_DIGEST_LENGTH);
    sizeText += MD5_DIGEST_LENGTH;
    request[sizeText++] = '/';
    
    memcpy(&request[sizeText], block, MD5_DIGEST_LENGTH);
    sizeText += MD5_DIGEST_LENGTH;
    
    request.resize(sizeText);
    std::cout << request << std::endl;
    return send(fd, request.c_str(), request.length(), 0);
}

int recvVal(std::vector<unsigned char*>& nodeHashs, int count, int index, int fd)
{
    std::string response(256, '\0');
    int F = count - 1 + index;
    int S = 0;
    std::vector<int> indexs;
    
    while (F != 0)
    {
        if (F % 2 == 0)
            S = F - 1;
        else
            S = F + 1;
        indexs.push_back(S);
        std::cout << "S = " << S << std::endl;
        F = (F - 1) / 2;
    }
    
    int size = indexs.size();
    const int countBlocks = size;
    struct iovec io[countBlocks];
    struct msghdr msg;
    unsigned char buffer[countBlocks][MD5_DIGEST_LENGTH];
    msg.msg_name = nullptr;
    
    for (int i = 0; i < size; ++i)
    {
        io[i].iov_base = buffer[i];
        io[i].iov_len = MD5_DIGEST_LENGTH;            
    }
    
    msg.msg_iov = io;
    msg.msg_iovlen = countBlocks;

    int len = recvmsg(fd, &msg, 0);
    std::cout << "SIZE = " << size << std::endl;
    for (int i = 0; i < size; ++i)
    {
        if (nodeHashs[indexs[i]] != nullptr)
            continue;
        nodeHashs[indexs[i]] = new unsigned char[MD5_DIGEST_LENGTH];
        memcpy(nodeHashs[indexs[i]],  buffer[i], MD5_DIGEST_LENGTH);
    }
    return size;
}

bool checkValidate(std::vector<unsigned char*>& nodeHashs, int index)
{
    int F = index; // 5
    int S = 0;
    bool f = false;
    while (index != 0) 
    {
        F = index; // 3
        if (F % 2 == 0)
            S = F - 1; // 
        else
            S = F + 1; // 4
        index = (index - 1) / 2; // 2

        std::cout << "F = " << F << std::endl;
        for(int k = 0; k < MD5_DIGEST_LENGTH; ++k)
            std::cout << int_to_hex((unsigned int)nodeHashs[F][k]) << " ";
        std::cout << std::endl;
        
        std::cout << "S = " << S << std::endl;
        for(int k = 0; k < MD5_DIGEST_LENGTH; ++k)
            std::cout << int_to_hex((unsigned int)nodeHashs[S][k]) << " ";
        std::cout << std::endl;

        MD5_CTX md5handler;
        MD5_Init(&md5handler);
        unsigned char* md5digest = new unsigned char[MD5_DIGEST_LENGTH];
        
        if (F < S)
        {
            MD5_Update(&md5handler, nodeHashs[F], MD5_DIGEST_LENGTH);
            MD5_Update(&md5handler, nodeHashs[S], MD5_DIGEST_LENGTH);            
            std::cout << "F < S" << std::endl;
        }
        else
        {
            MD5_Update(&md5handler, nodeHashs[S], MD5_DIGEST_LENGTH);
            MD5_Update(&md5handler, nodeHashs[F], MD5_DIGEST_LENGTH);
            std::cout << "F > S" << std::endl; 
        }
        std::cout << "###############" << std::endl;
                        
        
        MD5_Final(md5digest, &md5handler);
        

        std::cout << "INDEX = " << index << std::endl;

        for(int k = 0; k < MD5_DIGEST_LENGTH; ++k)
            std::cout << int_to_hex((unsigned int)md5digest[k]) << " ";
        std::cout << std::endl;
        
        if (nodeHashs[index] != nullptr && index != 0)
            delete [] md5digest;
        
        if (nodeHashs[index] == nullptr)
            nodeHashs[index] = md5digest;

        
        
        if (index == 0)
        {   
            if (memcmp(nodeHashs[index], md5digest, MD5_DIGEST_LENGTH) == 0)
            {
                std::cout << "Validate Ok" << std::endl;
                f = true;
            }
            else
            {
                std::cout << "Validate Bad" << std::endl;
                f = false;
            }
            delete [] md5digest;
        }
    }
    return f;
}

int main(int argc, char *argv[])
{
    std::string hostname;
    std::string portnum;
    std::string fullPath;
    std::string nameFile;
    std::string portServerString;
    int count = 0;
    int portServer = 0;
    int p = 0;
    int sizeBlock = SIZE;
    std::vector<unsigned char*> nodeHashs;
    std::vector<std::string> fileInBlocks;
    std::vector<int> emptyBlocks;
    std::vector<int> fillBlocks;
    int sizeFillBlocks = 0;
    unsigned char* merkleRoot = nullptr;
    if ( argc != 5 )
    {
        std::cout << "usage: " << argv[0] << " <hostname> <portnum> <filenamePath>\n" << std::endl;
        return 0;
    }
    
    hostname = argv[1];
    portnum = argv[2];
    fullPath = argv[3];
    portServerString = argv[4];
    portServer = std::stoi(portServerString);
    p = fullPath.find_last_of('/');
    nameFile = fullPath.substr(p + 1);
    signal(SIGINT, sigint);
        

    do
    {
        count = readDatabase(nodeHashs, nameFile);
        if (count == 0)
        {
            std::string request = "File/" + nameFile;
            std::string response(256, '\0');
            clientfd = openConnection(hostname.c_str(), std::stoi(portnum), clientfd);
            int len = send(clientfd, request.c_str(), request.length(), 0);
            if (len <= 0)
            {
                std::cout << "Error with send." << std::endl;
                break;
            }
            std::cout << "Request was send." << std::endl;
            len = recv(clientfd, &response[0], response.length(), 0);
            if (len <= 0)
            {
                std::cout << "Error with recv size message." << std::endl;
                break;
            }
            int sizeCommand = sizeof("File/") - 1;
            int pos = response.find('/', sizeCommand);
            if (pos == std::string::npos && pos + 1 < response.length())
            {
                std::cout << "Error with format response" << std::endl;
                break;
            }
            std::string sizeStr = response.substr(sizeCommand, pos - sizeCommand);
            count = std::stoi(sizeStr);
            std::cout << "Count = " << count << std::endl;

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
            
            nodeHashs[0] = new unsigned char[MD5_DIGEST_LENGTH];
            memcpy(nodeHashs[0], &response[pos + 1], MD5_DIGEST_LENGTH);

            // Empty check in fileInBlocks .........
            
            for (int i = 0; i < count; ++i)
                emptyBlocks.push_back(i);

            fileInBlocks.resize(count);
        }
        else
        {
            fileInBlocks.resize(count);
            sizeFillBlocks = readFile(fileInBlocks, fullPath);
            
            for (int i = 0; i < count; ++i)
            {
                if (fileInBlocks[i].size() == 0)
                    emptyBlocks.push_back(i);
                else
                    fillBlocks.push_back(i);
            }                        
        }
        int countBlocks = 1;
        while (countBlocks < count)
            countBlocks *= 2;
        merkleRoot = nodeHashs[0];
        serverfd = openListener(portServer);
        std::cout << "Size empty blocks = " << emptyBlocks.size() << std::endl;
        std::cout << "Size fill blocks = " << fillBlocks.size() << std::endl;        

        std::thread threadRecvRequest(recvRequest, std::ref(nodeHashs), std::ref(fileInBlocks), count, nodeHashs.size());

        if (emptyBlocks.size() == 0)
        {
            clientfd = openConnection(hostname.c_str(), std::stoi(portnum), clientfd);
            int len = sendSet(merkleRoot, "All/", portServer, clientfd);
            if (len <= 0)
            {
                std::cout << "Error with send size message." << std::endl;
                break;
            }
            
        }
        else
        {
            for (int i = 0; i < fillBlocks.size(); ++i)
            {
                clientfd = openConnection(hostname.c_str(), std::stoi(portnum), clientfd);
                int len = sendSet(merkleRoot, fillBlocks[i], portServer, clientfd);
                if (len <= 0)
                {
                    std::cout << "Error with send size message." << std::endl;
                    break;
                }
            }
            std::cout << "Port server on client = " << portServer << std::endl;
            
            for (int i = 0; i < emptyBlocks.size(); ++i)
            {
                clientfd = openConnection(hostname.c_str(), std::stoi(portnum), clientfd);
                int len = sendPort(merkleRoot, emptyBlocks[i], clientfd);
                if (len <= 0)
                {
                    std::cout << "Test 1" << std::endl;
                    std::cout << "Error with send size message." << std::endl;
                    break;
                }

                int portClient = recvPort(clientfd);

                if (portClient == 0)
                    break;

                if (portClient < 0)
                {
                    --i;
                    continue;
                }

                std::cout << "Port client with block = " << portClient << std::endl;
                clientfd = openConnection(hostname.c_str(), portClient, clientfd);                
                len = sendGetBlock(merkleRoot, emptyBlocks[i], clientfd);
                fileInBlocks[emptyBlocks[i]].resize(sizeBlock, '\0');
                g_lock.lock();
                len = recv(clientfd, &fileInBlocks[emptyBlocks[i]][0], sizeBlock, 0);
                g_lock.unlock();
                std::cout << "Recv block with size = " << len << std::endl;
                if (len <= 0)
                {
                    std::cout << "Test 2" << std::endl;
                    std::cout << "Error with send size message." << std::endl;
                    break;
                }
                bool f = false;
                g_lock.lock();
                fileInBlocks[emptyBlocks[i]].resize(len);
                if (fileInBlocks[emptyBlocks[i]] == "Get error")
                    f = true;
                g_lock.unlock();

                if (f)
                {
                    std::cout << "Error with get message." << std::endl;
                    break;
                }
                
                MD5_CTX md5handler;
                unsigned char* md5digest = new unsigned char[MD5_DIGEST_LENGTH];
                MD5_Init(&md5handler);
                for (int k = 0; k < len; k += MD5_DIGEST_LENGTH)
                {
                    int l = MD5_DIGEST_LENGTH;
                    if ((len - k) / MD5_DIGEST_LENGTH == 0)
                        l = len - k;
                    g_lock.lock();
                    MD5_Update(&md5handler, &fileInBlocks[emptyBlocks[i]][k], l);
                    g_lock.unlock();
                }
                MD5_Final(md5digest, &md5handler);

                g_lock.lock();
                nodeHashs[emptyBlocks[i] + countBlocks - 1] = md5digest;
                g_lock.unlock();

                clientfd = openConnection(hostname.c_str(), portClient, clientfd);
                
                len = sendVal("Val/", merkleRoot, md5digest, clientfd);
                if (len <= 0)
                {
                    std::cout << "Error with send size message." << std::endl;
                    break;
                }

                g_lock.lock();
                len = recvVal(nodeHashs, countBlocks, emptyBlocks[i], clientfd);
                g_lock.unlock();
                if (len <= 0)
                {
                    std::cout << "Test 4" << std::endl;
                    std::cout << "Error with send size message." << std::endl;
                    break;
                }
                
                f = true;
                g_lock.lock();
                if (!checkValidate(nodeHashs, emptyBlocks[i] + countBlocks - 1))
                    f = false; 
                g_lock.unlock();

                if (!f)
                {
                    std::cout << "Error with validate message." << std::endl;
                    break;
                }
                
                ++sizeFillBlocks;
                clientfd = openConnection(hostname.c_str(), std::stoi(portnum), clientfd);
                len = sendSet(merkleRoot, emptyBlocks[i], portServer, clientfd);
                if (len <= 0)
                {
                    std::cout << "Error with send size message." << std::endl;
                    break;
                }

                
                
            }
            //##############################################################################
        }
        threadRecvRequest.join();
    }
    while(0);
    
    shutdown(clientfd, SHUT_RDWR);
    if (sizeFillBlocks != 0)
        writeFile(fileInBlocks, fullPath, sizeFillBlocks);
    writeDatabase(nodeHashs, nameFile, count);
    std::cout << "Disconnect" << std::endl;
    return 0;
}
