#pragma once

#include <string>
#include <vector>
#include <queue>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <map>

#include "package.h"

using namespace std::chrono;

enum errors {
    ErrDublicate,
    ErrEmptyFileName,
    ErrCouldNotOpenFile,
    ErrCould
};

class FileBuilder {
public:
    FileBuilder(const std::string& dir, uint32_t marker);

    ~FileBuilder();

    void insert_package(Package&& package);

    void try_writing();

    bool file_is_ready() const;

    bool file_name_is_ready() const;

    std::string get_file_name() const;

    time_point<system_clock> get_last_writing_package_time() const;

    int complete();

private:
    uint32_t m_marker;
    uint32_t m_last_writed_pkg_number;
    bool  m_file_is_ready;
    bool  m_file_name_is_ready;
    std::string m_dir;
    std::string m_filename;
    time_point<system_clock> m_last_writing_package_time;
    std::priority_queue<Package> m_pkg_queue;
    std::ofstream m_fout;

    bool has_next() const;

    void initialize_file(const std::string& filename);
};