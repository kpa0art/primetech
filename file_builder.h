#ifndef FILE_BUILDER_H
#define FILE_BUILDER_H

#include <string>
#include <vector>
#include <queue>
#include <ctime>

#include "package.h"

#define TIMEOUT

class FileBuilder {
public:
    FileBuilder(uint32_t marker);

    void insert_package(Package& package);

    bool time_is_over() const;

private:
    uint32_t m_marker;
    uint32_t m_last_writed_pkg_number;
    std::time_t m_last_writing_time;
    std::priority_queue<Package, std::vector<Package>> m_pkg_queue;
    
    bool has_next() const;
};

#endif