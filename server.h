#pragma once

#include <string>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <vector>
#include <map>
#include <arpa/inet.h>

#include "package.h"
#include "file_builder.h"
#include "logger.h"

//#define DEBUG

using namespace std::chrono;

class Server
{
public:
    Server(const std::string& addr, int port, const std::string &dir, Logger& logger);

    ~Server();

    int get_socket() const;

    int get_port() const;

    std::string get_directory() const;

    std::string get_address() const;

    void work();

private:
    int m_socket;
    std::string m_dir;
    int m_port;
    std::string m_addr;
    Logger& m_logger;
    addrinfo *m_addrinfo;

    std::map<std::string, time_point<system_clock>> m_keys_black_list;
    std::map<std::string, std::unique_ptr<FileBuilder>> m_fb_store;
    std::vector<std::unique_ptr<Package>> m_pkg_store;
    
    void clear_file_builders_store_by_timeout();

    void clear_keys_black_list_by_timeout();

    bool allow_key(const std::string& key);

    FileBuilder* find_or_create_file_builder(const std::string& key);

    int timed_recvfrom(char *buf, int buf_len, sockaddr_in &addr, socklen_t &addr_len, int max_waiting_time_ms);

    int process_package(Package& package, const std::string& key);
};