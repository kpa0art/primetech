#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

class Logger: public std::ostream
{
public:
    Logger();

    Logger(const std::string& filename);

    bool file_is_open();

    void close();

private:
    class Buffer: public std::stringbuf
    {
    public:

        Buffer(Logger& logger);

        int sync();

    private:
        Logger& m_logger;
    };
    friend class Buffer;

    Buffer m_buffer;
    std::string m_filename;
    std::ofstream m_ofs;   
};