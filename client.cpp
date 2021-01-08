#include "client.h"

/** \brief Констуктор  клиента
 * 
 * Эта функция инициализирует объект клиента, принимая в качестве 
 * параметров адрес и порт сервера. Адрес сервера принимается в текстовом
 * виде в формате IPv4. Порт представлен в целочисленном типе. Если адрес
 * определится, будет сгенерировано исключение.
 * 
 * \warning
 * Используется только первый адрес, найденный функцией getaddrinfo.
 * 
 * \exception runtime_error
 * Клиент может проинициализироваться некоректно. Это может произойти
 * в случае, если не распознался адрес или порт не корректный или 
 * недоступный. 
 *  
 * \param[in] addr   IP адрес сервера в десятичном формате 
 * \param[in] port   Номер порта сервера
 */
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
    if (status != SUCCESS || m_addrinfo == nullptr)
    {
        throw std::runtime_error("некорректный адрес или порт");
    }
    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket < 0)
    {
        freeaddrinfo(m_addrinfo);
        throw std::runtime_error("не смог создать сокет");
    }
}

/** \brief Очистка объекта клиента
 * 
 * Функция освобождает структуру информации об адресе и закрывает сокет.
 */
Client::~Client()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}

/** \brief Возвращает копию адреса сервера 
 *
 * Функция возвращает копию адреса сервера, введенного при создании
 * клиента. Адрес не модифицируем, поэтому если требуется поменять адрес
 * необходимо создать нового клиента. 
 * 
 * \return Символьная копия адреса, переданного в конструктор.
 */  
std::string Client::get_address() const
{
    return m_addr;
}

/** \brief Возвращает порт сервера.
 * 
 * Функция возращает копию порта серврвера, введенного при создании 
 * клиента. Возвращаемое значение имеет целочисленный тип и не может 
 * быть изменено.
 * 
 *  \return Числовая значение порта сервера, переданного в конструктор.
 */ 
int Client::get_port() const
{
    return m_port;
}

/** \brief Получить копию индетификатора сокета.
 * 
 * Функция возвращает копию идентификатора сокета, присвоенного в процесе
 * инициализации клиента.
 * 
 * \return Возвращает целочисленное значения идентификатора сокета.
 */ 
int Client::get_socket() const
{
    return m_socket;
}

/** \brief Получить случайное значение.
 * 
 * Функция возвращает случайное значение, полученное с помощью стандарной
 * функции  rand().
 * 
 * \return Возвращает целое число.
 */ 
int get_random_value() {
    static auto seed = std::time(nullptr);
    std::srand(static_cast<unsigned int>(seed));
    return std::rand();
}

/** \brief Отправить данные через клиент. 
 * 
 * Функция отправляет данные из \p data в клиентский сокет по протоколу UDP.
 * 
 * \param[in] data    Данные для отправки.
 * \param[in] len     Длина данных в байтах. 
 * 
 * \return -1 , если в ходе выполен произошла ошибка. В таком случае
 * номер ошибки устанавливается в errno. При успешном выполнеии возращается
 * количество переданных байт.
 */ 
int Client::send(const char *data, int len) {

#ifdef DEBUG
    Package package(data, len);
    print_package_as_row(package);
#endif

    return sendto(m_socket, data, len, 0, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
}

/** \brief Очистка имени файла
 * 
 * Функция принимает на вход \p filename , в ктором может содержаться путь к
 * нему. Она отбрасывает путь к файлу, оставляя только имя файла.
 * 
 * \param[in] filename    Имя фала с указанным путем к нему.
 * 
 * \return Имя файла без пути.
 */ 
std::string clear_filename(const std::string filename)
{
    auto pos = filename.find_last_of("/");
    if (pos != std::string::npos)
        return filename.substr(pos+1);
    return filename;
}

/** \brief Отправка имени файла.
 * Функция отправляет имя файла. \p marker используется в идентификации
 * передаваемой информации в пределах одного отправителя.
 * 
 * \param[in] marker    Идентификатор файла.    
 * \param[in] filename  Имя отправляемого файла.
 * 
 * \return -1 , если в ходе выполения произошла ошибка. В таком случае
 * номер ошибки устанавливается в errno. При успешном выполнеии возращается
 * количество переданных байт.
 */ 
int Client::send_filename(uint32_t marker, const std::string &filename) 
{
    std::string cleared_filename = clear_filename(filename);
    Package package;
    package.set_number(1);
    package.set_marker(marker);
    package.set_data(cleared_filename.c_str(), strlen(cleared_filename.c_str()));
    return send(package.as_bytes(), package.package_size());
}

/** \brief Отправка содержимого файла.
 * Функция отправляет данные файла, получаемый  из потока \p in по частям.
 * \p marker используется в идентификации передаваемой информации в пределах
 * одного отправителя.
 * 
 * \param[in] marker    Идентификатор файла.    
 * \param[in] in        Входной поток данных файла.
 * 
 * \return -1 , если в ходе выполения произошла ошибка. В таком случае
 * номер ошибки устанавливается в errno. При успешном выполнеии возращается
 * количество переданных байт.
 */ 
int Client::send_file_data(uint32_t marker, std::ifstream& in) {
    Package package;
    package.set_marker(marker);
    char buf[MAX_DATA_SIZE];
    int buf_len = 0;
    uint32_t package_number = 1;
    int file_len = 0;
    do 
    {
        in.read(buf, std::streamsize(MAX_DATA_SIZE));
        buf_len = in.gcount();
        file_len += buf_len;
        package.set_number(++package_number);
        package.set_data(buf, buf_len);
        if (in.eof()) {
            package.set_package_flag(FLAG_LAST_PACKAGE);
        }
        if (send(package.as_bytes(), package.package_size()) < 0) {
            return -1;
        }
        // задержка требуется чтобы сервер успел прочитать переданные данные
        usleep(1000);
    } while (!in.eof());
    return file_len;
}

/** \brief Отправка файла.
 * 
 * Функция принимает имя файла в качестве \p filename , отрывает и передает 
 * имя файла и его содержимое по UDP протоколу.
 * 
 * \param[in] filename   Имя файла.    
 * 
 * \return -1 , если в ходе выполения произошла ошибка. В таком случае
 * номер ошибки устанавливается в errno. При успешном выполнеии возращается 0.
 */ 
int Client::send_file(const std::string &filename)
{
    std::ifstream ifs(filename, std::ios::binary | std::ios::in);
    if (ifs.fail())
        return -1;
    uint32_t marker = static_cast<uint32_t>(get_random_value());

#ifdef DEBUG
    print_headers_as_row();
#endif

    if (send_filename(marker, filename) < 0) 
    {
        ifs.close();
        return -1;
    }    
    if (send_file_data(marker, ifs) < 0)
    {
        ifs.close();
        return -1;
    }
    ifs.close();
    return 0;
}

void print_usage(char *program_name)
{
    std::cout << "Используйте: " << program_name;
    std::cout << " <IPv4 адрес сервера> <Порт сервера>"
                 " <Имя файла>"
              << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "ошибка: требуется четыре аргумента" << std::endl;
        print_usage(argv[0]);
        exit(1);
    }
    int port = 0;
    try 
    {
        port = std::stoi(std::string(argv[2]));
    }
    catch (std::invalid_argument &e)
    {
        std::cerr << "Ошибка: значение порта должено быть целом числом." << std::endl;
        exit(1);
    }
    try
    {
        std::cout << "Инициализация клиента: ";
        Client client(std::string(argv[1]), port);
        std::cout << "Успешно." << std::endl << "Попытка передачи фала \"" 
            << argv[3] << "\" по адресу [" << client.get_address() << ":" 
            << client.get_port() << "]" << std::endl; 
        
        if (client.send_file(std::string(argv[3])) < 0)
        {
            std::cerr << "Отправка не удалась. Ошибка: " 
                << std::strerror(errno) << std::endl;
            exit(1);
        }
        std::cout << "Отправка произведена успешно." << std::endl;
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << "Ошибка при инициализации клиента: " << err.what() 
            << std::endl;
        exit(1);
    }
    return 0;
}