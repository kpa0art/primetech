#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>

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

private:
    int m_socket;
    int m_port;
    std::string m_addr;
    addrinfo *m_addrinfo;

    int send(const char *data, int len);
    
    // void pack_package_number(char *buf, int *len, int32_t pkg_num);
    // void pack_package_marker(char *buf, int *len, int32_t marker);
    // void pack_data(char *buf, int *buf_len, char *data, int data_len);
};

#endif