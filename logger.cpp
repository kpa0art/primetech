#include "logger.h"
#include "format.h"

#include <ctime>
#include <string>

Logger::Logger()
    : std::ostream(&m_buffer)
    , m_buffer(*this)
{}

Logger::Logger(const std::string& file_name) 
    : std::ostream(&m_buffer)
    , m_buffer(*this)
    , m_file_name(file_name)
    , m_ofs(file_name.c_str()) 
{}

bool Logger::file_is_open()
{
    return m_ofs.is_open();
}

void Logger::close()
{
    m_ofs.close();
}

Logger::Buffer::Buffer(Logger& logger)
    : m_logger(logger)
{}

int Logger::Buffer::sync()
{
    auto ts = std::time(0);
    std::stringstream ss;
    ss << "[" << std::localtime(&ts) << "]" << str();
    std::cout << ss.str();
    if (m_logger.m_ofs.is_open())
        m_logger.m_ofs << ss.str();
    str("");
    return 0;
}