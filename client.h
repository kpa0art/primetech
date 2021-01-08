#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>

#include "common.h"
#include "package.h"



class Client
{
public:

    Client(const std::string &addr, int port);

    ~Client();

    std::string get_address() const;

    int get_port() const;

    int get_socket() const;

    int send_file(const std::string& filename );

    char* strerror(int result);

private:
    int m_socket;
    int m_port;
    std::string m_addr;
    addrinfo *m_addrinfo;
    std::ifstream ifs;

    int send(const char *data, int len);

    int send_filename(uint32_t marker, const std::string& filename);

    int send_file_data(uint32_t marker, std::ifstream& ifs);
};

#endif