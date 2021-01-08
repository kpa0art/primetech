#include "logger.h"
#include "format.h"

#include <ctime>
#include <string>

/** \brief Конструктор объекта логгера
 * 
 * Функция инициализирует объект логера.
 * 
 * \note Функция 
 */ 
Logger::Logger()
    : std::ostream(&m_buffer)
    , m_buffer(*this)
{}

/** \brief Конструктор объекта логгера
 * 
 * Функция инициализирует объект логера. Так же осуществляется попытка 
 * открытия файла \p filename. Проверить, открыт ли файл файл можно методом
 * file_is_open().
 */ 
Logger::Logger(const std::string& filename) 
    : std::ostream(&m_buffer)
    , m_buffer(*this)
    , m_filename(filename)
    , m_ofs(filename.c_str()) 
{}

/** \brief Проверка открытия файла.
 * 
 * Функция проверяет, открыт ли файл, имя которого передано в конструкторе  
 * Logger(const std::string& filename).
 */ 
bool Logger::file_is_open()
{
    return m_ofs.is_open();
}

/** \brief Закрытие файла, в который пишется лог. 
 * 
 * Функция закрывает файл, открытый при Logger(const std::string& filename).
 */
void Logger::close()
{
    m_ofs.close();
}

/** \brief Конструктор буфера. 
 * 
 * Функция инициализирует объект буфера, в которой передаются все строки,
 * переданные через Logger.
 * 
 * \note
 * Buffer является дружественым классом Logger, поэтому ему необходимо
 * передавать в качестве параметра ссылку \p logger для доступа к приватным
 * параметрам объекта Logger. 
 */
Logger::Buffer::Buffer(Logger& logger)
    : m_logger(logger)
{}

/** \brief Синхронизировать символы в буфере. 
 *
 * Функция перенаправляет сохраненные символы в консольный вывод, добавляя
 * вначале временную метку. Если в Logger есть открытый файл, то он дублирует
 * вывод в этот файл.
 * 
 * \note
 * Так как Buffer являтся наследником streambuf, требовалось переопределить 
 * виртуальную функцию sync. Функция не предполагает ошибок в ходе выполнения,
 * поэтому всегда возращает 0. 
 * 
 * \return Всегда возращает 0.
 */
int Logger::Buffer::sync()
{
    auto ts = std::time(0);
    std::stringstream ss;
    ss << "[" << std::localtime(&ts) << "]" << str();
    std::cout << ss.str();
    if (m_logger.m_ofs.is_open())
        m_logger.m_ofs << ss.str();
    str(std::string());
    return 0;
}