#include "file_builder.h"

FileBuilder::FileBuilder(uint32_t marker)
    : m_marker(marker)
    , m_last_writed_pkg_number(-1)
    , m_last_writing_time(0)
{
}

bool FileBuilder::has_next() const 
{
    if (m_pkg_queue.empty()) {
        return false;
    }
    return m_pkg_queue.top().get_number() - m_last_writed_pkg_number == 1;
}

void FileBuilder::insert_package(Package& package) {
    assert(package.get_marker() == m_marker);
    m_pkg_queue.push(package);
}

bool FileBuilder::time_is_over() const
{
    return std::difftime(std::time(NULL), m_last_writing_time);
}