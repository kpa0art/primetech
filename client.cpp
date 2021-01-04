#include "client.h"

Client::Client(const std::string &addr, int port)
    : m_port(port)
    , m_addr(addr)
{
    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_protocol = IPPROTO_UDP;
    std::string s_port= std::to_string(port);
    int status = getaddrinfo(addr.c_str(), s_port.c_str(), &hint, &m_addrinfo);
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

int Client::send(const char *buf, int len) {

    Package package(buf, len);
    print_package_as_row(package);

    return sendto(m_socket, buf, len, 0, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
}

int Client::send_file_name(uint32_t marker, const std::string &file_name) {
    Package package;
    package.set_number(1);
    package.set_marker(marker);
    package.set_data(file_name.c_str(), strlen(file_name.c_str()) + 1);
    return send(package.as_bytes(), package.package_size());
}

int Client::send_file_data(uint32_t marker, std::ifstream& ifs) {
    Package package;
    package.set_marker(marker);
    std::vector<char> data;
    data.reserve(MAX_DATA_SIZE);
    uint32_t package_number = 1;
    while (!ifs.eof())
    {
        ifs.read(data.data(), MAX_DATA_SIZE);
        data.resize(ifs.gcount());
        package.set_number(++package_number);
        package.set_data(data.data(), data.size());
        if (ifs.eof()) {
            package.set_package_flag(FLAG_LAST_PACKAGE);
        }
        if (send(package.as_bytes(), package.package_size()) < 0) {
            return FAILED;
        }
    }
    return SUCCESS;
}

int Client::send_file(const std::string &file_name)
{
    std::ifstream ifs(file_name, std::ios::binary | std::ios::in);
    if (ifs.fail())
        return FAILED;
    uint32_t marker = 12345678;
    print_headers_as_row();
    if (send_file_name(marker, file_name) < 0) 
    {
        ifs.close();
        return FAILED;
    }    
    if (send_file_data(marker, ifs) < 0)
    {
        ifs.close();
        return FAILED;
    }
    ifs.close();
    return SUCCESS;
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
        if (client.send_file(std::string(argv[3])) != SUCCESS)
            std::cerr << strerror(errno) << std::endl;
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what();
        exit(1);
    }

    return 0;
}