#include "server.h"
#include "tool.h"

#include <cstring>

const std::chrono::seconds black_list_timeout(30);        // 30 секунд игнорирования входящих пакетов
const std::chrono::seconds max_package_waiting_time(10);  // 10 секунд ожидания следующего необходимого пакета
                                                          // для записи

Server::Server(const std::string &addr, int port, const std::string &dirname, Logger& logger)
    : m_dir(dirname)
    , m_port(port)
    , m_addr(addr)
    , m_logger(logger)
{
    if (!dir_exists(dirname))
        throw std::runtime_error("directory does not exists");
    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_protocol = IPPROTO_UDP;
    std::string s_port = std::to_string(port);
    int status = getaddrinfo(addr.c_str(), s_port.c_str(), &hint, &m_addrinfo);
    if (status != SUCCESS || m_addrinfo == nullptr)
        throw std::runtime_error("invalid address or port");
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

int extract_address_info(const sockaddr_in &address, std::string &ip, int &port) {
    char buf[INET_ADDRSTRLEN];
    port = ntohs(address.sin_port);
    if (inet_ntop(AF_INET, &(address.sin_addr), buf, INET_ADDRSTRLEN) == nullptr)
        return errno;
    ip = buf;
    return SUCCESS;
}

std::string make_key(std::string& client_ip, int client_port, uint32_t marker)
{
    std::ostringstream  ss;
    ss << client_ip << "-" << client_port << "-" << static_cast<unsigned long>(marker);
    return ss.str();
}

void unmake_key(const std::string& key, std::string& ip, int& port, uint32_t& marker) 
{
    auto first_pos = key.find_first_of("-");
    if (first_pos == std::string::npos)
        return;
    auto last_pos = key.find_last_of("-");
    if (last_pos == std::string::npos)
        return;
    if (first_pos == last_pos)
        return;
    ip = key.substr(0, first_pos);
    port = std::stoi(key.substr(first_pos + 1, last_pos - first_pos + 1));
    marker = static_cast<uint32_t>(std::stoul(key.substr(last_pos + 1)));
}

void Server::clear_file_builders_store_by_timeout()
{
    auto now = std::chrono::system_clock::now();
    for (auto iter = m_fb_store.begin(); iter != m_fb_store.end();) {
        FileBuilder *fb = iter->second.get();
        std::string key = iter->first;
        if (fb->get_last_writing_package_time() - now > max_package_waiting_time)
        {
            int port;
            uint32_t marker;
            std::string ip;
            std::string file_name;
            if (fb->file_name_is_ready())
                file_name = fb->get_file_name();
            unmake_key(key, ip, port, marker);
            m_logger << "[INFO] deleting file ("
                << ((file_name != "") ? file_name : "Unknown") << ") "
                <<" from ["  << ip << ":" << port << "]" 
                << "by timeout" << std::endl;
            m_keys_black_list[key] = now;
            iter = m_fb_store.erase(iter);
            continue;
        } else {
            iter++;
        }   
    }        
}

bool Server::allow_key(const std::string& key)
{
    auto now = std::chrono::system_clock::now();
    auto iter_bl = m_keys_black_list.find(key);
    if (iter_bl != m_keys_black_list.end()) 
    {
        if (iter_bl->second - now <= black_list_timeout)
        {
            iter_bl->second = now;
            return false;
        }
        m_keys_black_list.erase(iter_bl);        
    }
    return true;
}

FileBuilderStore::iterator Server::find_or_create_file_builder(const std::string& key)
{
    auto iter_store = m_fb_store.find(key);
    if (iter_store == m_fb_store.end())
    {
        int port;
        uint32_t marker;
        std::string ip;
        unmake_key(key, ip, port, marker);
        m_logger << "[INFO] incoming new file from: [" 
            << ip << ":" << port << "]" << std::endl; 
        iter_store = m_fb_store.emplace(
            key,
            std::make_unique<FileBuilder>(m_dir, marker)
        ).first;
    }
    return iter_store;
}

int Server::wait() 
{
    char buf[MAX_PACKAGE_SIZE];
    sockaddr_in addr;
    socklen_t   addr_len = sizeof(sockaddr);
    memset(&addr, 0, sizeof(sockaddr));

    std::string client_ip;
    int client_port = 0;
    std::ostringstream  ss;

    print_headers_as_row();

    while(1) {
        int bytes_len = recvfrom(m_socket, buf, MAX_PACKAGE_SIZE, 0, (sockaddr*)&addr, &addr_len);
        if (bytes_len < 0) {
            m_logger << "[ERROR] " << strerror(errno) << std::endl;
            continue;
        }
        if (extract_address_info(addr, client_ip, client_port) != SUCCESS) {
            m_logger << "[ERROR] " << strerror(errno) << std::endl;
            continue;
        }
        Package package(buf, bytes_len);
        print_package_as_row(package);
        if (!package.valid())
        {
            m_logger << "[WARNING] incoming bad package from [" 
                << client_ip << ":" << client_port << "]" << std::endl;
            continue;
        }
        uint32_t marker = package.get_marker();
        std::string key = make_key(client_ip, client_port, marker);
        
        if (!allow_key(key))
            continue;
        auto iter = find_or_create_file_builder(key);
        FileBuilder *fb = iter->second.get();
        fb->insert_package(std::move(package));
        if (!fb->exists())
        {
            if (!fb->can_create())
                continue;
            if (fb->create() < 0) {
                m_logger << "[ERROR] could not create file \"" 
                    << fb->get_file_name() << "\"" << std::endl;
            }
        } else if ((int result = fb->try_writing()) != SUCCESS) {
            if (result == ErrEcxeptPackage)
                continue;
            m_logger << "[ERROR] could not write to file \"" 
                << fb->get_file_name() << "\"" << std::endl;    
        } else if (fb->is_ready()) {
            if (fb->complete() != SUCCESS) {
                m_logger << "[ERROR] could not finish file. \"" 
                    << fb->get_file_name() << "\" from [" 
                    << client_ip << ":" << client_port << "]" << std::endl;    
            }
            m_logger << "[INFO] recived and saved file \"" 
                << fb->get_file_name() << "\" from [" 
                << client_ip << ":" << client_port << "]" << std::endl;
            m_fb_store.erase(iter);
        }



        fb->insert_package(std::move(package));
        try{
            fb->try_writing();
            if (fb->file_is_ready())
            {
                fb->complete();
                m_logger << "[INFO] recived and saved file \"" 
                    << fb->get_file_name() << "\" from [" 
                    << client_ip << ":" << client_port << "]" << std::endl;
                m_fb_store.erase(iter);
            }
        }
        catch (const std::runtime_error& e) {
            m_logger << "[ERROR] " << e.what() << std::endl;
            m_keys_black_list.emplace(key, std::chrono::system_clock::now());
            m_fb_store.erase(iter);
        }
        
        clear_file_builders_store_by_timeout();
    }
    return SUCCESS;
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
    Logger log;
    try
    {
        Server server(std::string(argv[1]), port, argv[3], log);
        if (server.wait() < 0)
            std::cerr << strerror(errno) << std::endl;
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what();
        exit(1);
    }
    return 0;
}


