#include "server.h"

Server::Server(const std::string &addr, int port, const std::string &dirname)
    : m_dir(dirname), m_port(port), m_addr(addr)
{
    // if (!dir_exists()) {
    //     make_dir();
    // }

    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_protocol = IPPROTO_UDP;
    char port_cstr[10];
    snprintf(port_cstr, sizeof(port_cstr), "%d", port);
    int status = getaddrinfo(addr.c_str(), port_cstr, &hint, &m_addrinfo);
    if (status != SUCCESS || m_addrinfo == NULL)
    {
        throw std::runtime_error("invalid address or port");
    }
    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET)
    {
        freeaddrinfo(m_addrinfo);
        throw std::runtime_error("could not create socket\n");
    }
    status = bind(m_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if ( status != SUCCESS ) {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        throw std::runtime_error(strerror(errno));
    }
}

Server::~Server()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}

std::string Server::get_address() const
{
    return m_addr;
}

int Server::get_port() const
{
    return m_port;
}

int Server::get_socket() const
{
    return m_socket;
}

std::string Server::get_directory() const {
    return m_dir;
}

void Server::wait() 
{
    char buf[MAX_PACKAGE_SIZE];
    sockaddr addr;
    socklen_t addr_len;
    while(1) {
        int bytes = recvfrom(m_socket, buf, MAX_PACKAGE_SIZE, 0, &addr, &addr_len);
        if (bytes < 0) {
            throw std::runtime_error("shit happen");
        }
        Package pkg(buf, bytes);
        //pkg.info();
        
    }
}

void print_usage(char *program_name)
{
    std::cout << "Use: " << program_name;
    std::cout << " <server addres> <server port> <store dirname>" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "error: few arguments" << std::endl;
        print_usage(argv[0]);
        exit(1);
    }
    int port = std::stoi(std::string(argv[2]));
    try
    {


        Server server(std::string(argv[1]), port, argv[3]);
        server.wait();
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what();
        exit(1);
    }

    return 0;
}


