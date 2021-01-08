#include "server.h"

#include <cstring>
#include <sys/stat.h>

static const std::chrono::seconds key_black_list_timeout(30);   // 30 секунд игнорирования входящих пакетов по ключу
static const std::chrono::seconds max_package_waiting_time(5);  // 2 секунд ожидания следующего необходимого пакета
                                                                // для записи

/** \brief Проверка существования директории
 * 
 * Функция проверяет существование директори с именем  \p dirname .
 * 
 * \return true, если директория существует, false иначе.
 */ 
bool dir_exists(const std::string& dirname)
{
    struct stat info;
    if (stat(dirname.c_str(), &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

/** \brief Функция создания UDP сервера.
 * 
 * Эта функция создает объект сервера, принимая в качестве параметров 
 * етевой адрес  интерфеса и порт, с которого сервер будет ожидать данные.
 * Адрес \p addr принимается в текстовом виде в формате IPv4. Порт \p port 
 * представлен в целочисленном типе. Если адрес не определится, будет
 * сгенерировано исключение. Третьим парамтром является директория \p dirname , 
 * включая полный путь к ней, в котороую будет сохраняться файлы, переданные
 * по протоколу UDP.
 * 
 * \warning
 * Используется только первый адрес, найденный функцией getaddrinfo.
 * 
 * \exception runtime_error
 * Сервер может проинициализироваться некоректно. Это может произойти
 * в случае, если не распознался адрес, порт оказался не корректный или 
 * недоступный, не найдена указанная директория, не удалось создать UDP сокет,
 * не удалось привязать адрес к сокету.
 * 
 * \param[in] addr   IP адрес сервера в десятичном формате
 * \param[in] port   Номер порта сервера в виде целого числа.
 */ 
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

/** \brief Деструктор UDP сервера.
 * 
 * Функция освобождает ресурсы объекта сервера.
 */  
Server::~Server()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}

/** \brief Возращает копию адреса сервера.
 * 
 * Функция возращает копию адреса сервера, переданного в качестве параметра
 * конструктора класса.
 * 
 * \note
 * Адрес сервера нельзя изменить. Если требуется изменить адрес сервера, то 
 * требуется создать новый объект сервера. 
 */ 
std::string Server::get_address() const
{
    return m_addr;
}

/** \brief Возращает копию порта сервера.
 * 
 * Функция возращает копию порта сервера, переданного в качестве параметра
 * конструктора класса.
 * 
 * \note
 * Порт сервера нельзя изменить. Если требуется изменить порт сервера, то 
 * требуется создать новый объект сервера. 
 */ 
int Server::get_port() const
{
    return m_port;
}

/** \brief Получить копию индетификатора сокета.
 * 
 * Функция возвращает копию идентификатора сокета, присвоенного в процесе
 * инициализации сервера.
 * 
 * \return Возвращает целочисленное значения идентификатора сокета.
 */ 
int Server::get_socket() const
{
    return m_socket;
}

/** \brief Получить копию директории.
 * 
 * Функция возвращает копию директории, введенной в процесе инициализации 
 * сервера. В эту дирректорию сохраняются все переданные файлы.
 *
 * \return Возвращает копию строки, содержащий директроию.
 */ 
std::string Server::get_directory() const {
    return m_dir;
}

/** \brief Извлечь информацию об адресе.
 * 
 * Функция извлекает информацию из параметра \p address (cnhernehf sockaddr_in), 
 * и загружает IPv4 адрес в \p ip и номер порта в \p port.
 * 
 * \param[in] address    Заполенная структура адреса sockaddr_in.
 * \param[in] ip         Строка, в которую будет загружен IPv4 адрес.
 * \param[in] iport      Порт, в который будет загружена информациия из address 
 * 
 * \return Возвращает копию строки, содержащий директроию.
 */ 
int extract_address_info(const sockaddr_in &address, std::string &ip, int &port) {
    char buf[INET_ADDRSTRLEN];
    port = ntohs(address.sin_port);
    if (inet_ntop(AF_INET, &(address.sin_addr), buf, INET_ADDRSTRLEN) == nullptr)
        return errno;
    ip = buf;
    return SUCCESS;
}

/** \brief Создать ключ из имеющихся парамтров.
 * 
 * Функция создает строковый ключ, используя IP и порт клиента, идентификатор
 * потока пакетов. Ключ получается относительно уникальным.
 * 
 * \note
 * Ключ имеет вид: <clint_ip>-<client_port>-<marker>.
 * 
 * \param[in] client_ip    Адрес клиента
 * \param[in] client_port  Порт клиента.
 * \param[in] marker       Идентификатор потока пакета.
 * 
 * \return Возвращает копию строки, содержащий созданный ключ.
 */ 
std::string make_key(std::string& client_ip, int client_port, uint32_t marker)
{
    std::ostringstream  ss;
    ss << client_ip << "-" << client_port << "-" << static_cast<unsigned long>(marker);
    return ss.str();
}

/** \brief Разобрать ключ.
 * 
 * Функция разбирает ключ, созданный на онсове параметров сетевого адреса, 
 * порта и идентификатора потока и загружает его в парамтры \p ip , \p port ,
 * \p marker . Эти параметры использовались при создании ключа в make_key. 
 * 
 * \note
 * Ключ имеет вид: <clint_ip>-<client_port>-<marker>.
 * 
 * \param[in] key       Строка ключ.
 * \param[in] ip        строка адрес клиента.
 * \param[in] port      Порт клиента.
 * \param[in] marker    Идентификатор потока пакетов.
 * 
 * \return Возвращает копию строки, содержащий созданный ключ.
 */ 
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

/** \brief Удалить сборщик файла по таймауту.
 * 
 * Функция удаляет сборщик файла в который уже долгое время не приходил пакет.
 */ 
void Server::clear_file_builders_store_by_timeout()
{
    auto now = std::chrono::system_clock::now();
    for (auto iter = m_fb_store.begin(); iter != m_fb_store.end();) {
        FileBuilder *fb = iter->second.get();
        std::string key = iter->first;
        if (now - fb->get_last_writing_package_time() > max_package_waiting_time ||
            fb->file_is_ready())
        {
            int port;
            uint32_t marker;
            std::string ip;
            unmake_key(key, ip, port, marker);
            std::string file_name;
            if (fb->file_name_is_ready())
                file_name = fb->get_file_name();
            if (fb->file_is_ready())
            {
                m_logger << "[INFO] Получен файл \""
                    << file_name << "\" из ["  << ip << ":" << port << "]" 
                    << std::endl;
            } else 
            {
                m_logger << "[INFO] удален файл \""
                    << ((file_name != "") ? file_name : "Unknown") 
                    << "\" по таймауту " <<" от ["  << ip << ":" 
                    << port << "]" << std::endl;
            }
            m_keys_black_list[key] = now;
            iter = m_fb_store.erase(iter);
            continue;
        } else {
            iter++;
        }   
    }        
}

/** \brief Удалить ключ из черно листа по таймауту
 * 
 * Функция удаляет ключ ключ из черного листа по таймауту, время таймаута 
 * определено в переменной key_black_list_timeout.
 */ 
void Server::clear_keys_black_list_by_timeout()
{
    auto now = std::chrono::system_clock::now();
    for (auto iter = m_keys_black_list.begin(); iter != m_keys_black_list.end();) {
        if (now - iter->second > key_black_list_timeout)
            iter = m_keys_black_list.erase(iter);
        else {
            iter++;
        }
    }
}

/** \brief Проверка на разрешеный ключ
 * 
 * Функция проверяет, находится ли ключ в черном списке. В случае, если ключ
 * помещен в этот список, то время срок его нахождения в этом списке 
 * обновляется и составляет key_black_list_timeout.
 * 
 * \param[in] key    Символьный ключ.
 * 
 * \return true, если ключ не находится в черном списке, false иначе.
 */ 
bool Server::allow_key(const std::string& key)
{
    auto now = std::chrono::system_clock::now();
    auto iter_bl = m_keys_black_list.find(key);
    if (iter_bl != m_keys_black_list.end()) 
    {
        if (iter_bl->second - now <= key_black_list_timeout)
        {
            iter_bl->second = now;
            return false;
        }
        m_keys_black_list.erase(iter_bl);        
    }
    return true;
}

/** \brief Нахождение или создание сборщика файла
 * 
 * Функция возращает указатель на файловый сборщик. В случае, если сборщик
 * не найден то будет создан сборщик по ключу \p key. 
 * 
 * \param[in] key    Символьный ключ.
 * 
 * \return           Указатель на объект файловорго сборщика. 
 */ 
FileBuilder* Server::find_or_create_file_builder(const std::string& key)
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
    return iter_store->second.get();
}

/** \brief Ограниченный по времени recvfrom
 * 
 * Функция ограничевает по времени блокировку функцией recvfrom. В работе 
 * функции используются вызовы recvfrom и select, поэтому в случае
 * возникновения ошибки в ходе выполнения функция вернет -1, а соответствующая
 * ошибка будет установлена в errno. 
 * 
 * \param[in] buf                  Массив байтов, куда будет записана информация
 * \param[in] buf_len              Размер массива байтов.
 * \param[in] addr                 Адрес, с которого пришли данные.
 * \param[in] addr_len             Длинна адреса.
 * \param[in] max_waiting_time_ms  Максимальное время ожидания работы recvfrom
 * 
 * \return -1, в случае ошибки или количество принятых байтов. 
 */ 
int Server::timed_recvfrom(char *buf, int buf_len, sockaddr_in &addr, socklen_t &addr_len, int max_waiting_time_ms)
{
    fd_set s;
    FD_ZERO(&s);
    FD_SET(m_socket, &s);
    struct timeval timeout;
    timeout.tv_sec = max_waiting_time_ms / 1000;
    timeout.tv_usec = (max_waiting_time_ms % 1000) * 1000;
    int result = select(m_socket + 1, &s, 0, 0, &timeout);
    if (result < 0)
        return -1;
    else if (result == 0)
    {
        errno = EAGAIN;
        return -1;
    }
    return recvfrom(m_socket, buf, buf_len, 0, (sockaddr*)&addr, &addr_len);
}

/** \brief Обработка пакета
 * 
 * Функция обрабатывает пакет \p package , поставляемый вместе с ключом \p key .
 * в случае, если ключ разрешен, то пакет доставляется файловому сборщику для 
 * последующей обработки им. Если не разрешен, то пакет удаляется. Вслучае 
 * возникновения ошибки, ключ будет добавлен в черный список и все последующие
 * пакеты с этим ключем будут проигнорированы.
 * 
 * \note
 * Как составляется ключ смотрите в функции make_key.
 * 
 * \param[in] package   Пакет данных файла.
 * \param[in] key       Строковый ключ.
 * 
 * \return 0, в случае успешного выполнения,В противном случае возвращается 
 * ошибка, код которых определенн функцией FileBuilder::process.
 */ 
int Server::process_package(Package& package, const std::string& key)
{
    if (!allow_key(key))
        return 0;
    FileBuilder *fb = find_or_create_file_builder(key);
    fb->insert_package(std::move(package));
    int result = fb->process();
    if (result != 0 && result != ErrExpectPackage) {
        m_keys_black_list.emplace(key, system_clock::now());
    } 
    return result;
}

/** \brief Работа сервера
 * 
 * Функция запускает бесконечный процесс ожидания пакетов от клиентов с 
 * последующей их обработкой. После каждого цикла происходит 
 * очистка хранилища с файловыми сборщиками и очиска черного листа с ключами
 * по таймауту. В случае возникновения ошибок при вызове функций в теле функции
 * происходит их логиирование в Logger, переданный при инициализации 
 * конструктора сервера.
 * 
 * \warning
 * Функция работает медленно, поэтому если несколько клиентов будут передавать 
 * свои файлы, то сервер не будет успеваь считать все пакеты, что приведет к
 * ситуации, когда сервер не сможет принять ни один файл клиентов, так как 
 * некоторые пакеты будут утеряны.
 */ 
void Server::work() 
{
    char buf[MAX_PACKAGE_SIZE];
    sockaddr_in addr;
    socklen_t   addr_len = sizeof(sockaddr_in);

    std::string client_ip;
    int client_port = 0;

#ifdef DEBUG
    print_headers_as_row();
#endif

    m_logger << "[INFO] Начат прием паветов. " << std::endl;
    while(1) {
        memset(&addr, 0, sizeof(sockaddr_in));
        int bytes = timed_recvfrom(buf, MAX_PACKAGE_SIZE, addr, addr_len, 2000);
        if (bytes < 0) {
            if (errno != EAGAIN)
                m_logger << "[ERROR] " << strerror(errno) << std::endl;
        } else {

            extract_address_info(addr, client_ip, client_port);
            Package package(buf, bytes);

#ifdef DEBUG            
            print_package_as_row(package);
#endif
            if (!package.valid())
            {
                m_logger << "[WARNING] incoming bad package from [" 
                    << client_ip << ":" << client_port << "]" << std::endl;
                continue;
            }
            std::string key = make_key(client_ip, client_port, package.get_marker());
            int result = process_package(package, key);
            if (result != 0 && result != ErrExpectPackage)
            {
                
                if (result == ErrErrno) {
                    m_logger << "[ERROR] ошибка ["<< errno << "]:"
                        << strerror(errno) << ". client: [" << client_ip 
                        << ":" << client_port << "]" << std::endl;     
                } else if (result == ErrInvalidFileName) {
                    m_logger << "[ERROR] Пришло невалидное имя файла из: [" 
                        << client_ip << ":" << client_port << "]" << std::endl;                     
                } else if (result == ErrCouldNotCreateFile) {
                    m_logger << "[ERROR] Не смог созать файл [" << result << "]: " 
                        << strerror(errno) << std::endl;
                } else {
                    m_logger << "[ERROR] Unknown error" << std::endl;
                }
            }
        }
        clear_file_builders_store_by_timeout();
        clear_keys_black_list_by_timeout();
    }
}

void print_usage(char *program_name)
{

    std::cout << "Используйте: " << program_name;
    std::cout << " <IPv4 адрес сервера> <Порт> <Директория для хранения файлов>" 
        << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Ошибка: неверное количество аргументов." << std::endl;
        print_usage(argv[0]);
        exit(1);
    }
    int port = std::stoi(std::string(argv[2]));
    Logger log;
    try
    {
        Server server(std::string(argv[1]), port, argv[3], log);
        server.work();
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what();
        exit(1);
    }
    return 0;
}


