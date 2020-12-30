#include "client.h"

Client::Client(const std::string &addr, int port)
    : m_port(port), m_addr(addr)
{
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
}

Client::~Client()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}

std::string Client::get_address() const
{
    return this->m_addr;
}

int Client::get_port() const
{
    return m_port;
}

int Client::get_socket() const
{
    return m_socket;
}

int Client::send_file(const std::string &filename)
{
    const char *name = filename.c_str();
    uint32_t namelen = strlen(name) + 1;
    if (namelen > MAX_DATA_SIZE) {
        throw std::runtime_error("filename very big\n");
    }
    std::fstream fs;
    fs.open(name, std::fstream::in);
    if ( !fs) {
        throw std::runtime_error("file does not exist\n");
    }
    fs.close();
    
    Package package;
    package.set_number(1);
    package.set_marker(uint32_t(12345678));
    package.set_data(name, namelen);
    package.info();
    return send(package.as_bytes(), package.package_size());
    // return send("hello", 6);
}

int Client::send(const char *buf, int len) {
    return sendto(m_socket, buf, len, 0, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
}

void print_usage(char *program_name)
{
    std::cout << "Use: " << program_name;
    std::cout << " <destination server addres> <destination server port>"
                 " <filename>"
              << std::endl;
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
        Client client(std::string(argv[1]), port);
        int bytes = client.send_file(std::string(argv[3]));
        if ( bytes < 0 ) {
            std::cout << "fuck...";
        }
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what();
        exit(1);
    }

    return 0;
}