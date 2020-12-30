#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "common.h"
#include "package.h"

class Server
{
public:
    Server(const std::string& addr, int port, const std::string &dir);

    ~Server();

    int get_socket() const;

    int get_port() const;

    std::string get_directory() const;

    std::string get_address() const;

    void wait();

private:
    int m_socket;
    std::string m_dir;
    int m_port;
    std::string m_addr;
    addrinfo *m_addrinfo;
    
    // bool dir_exists();
    // void make_dir();

};

#endif