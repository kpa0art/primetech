#include "file_builder.h"

#include <cstdio>
#include <sys/stat.h>

static const std::string downloadble_file_ext(".downloadble");

FileBuilder::FileBuilder(const std::string& dir, uint32_t marker)
    : m_marker(marker)
    , m_last_writed_pkg_number(0)
    , m_file_is_ready(false)
    , m_file_name_is_ready(false)
    , m_dir(dir)
    , m_last_writing_package_time(system_clock::now())
{}

FileBuilder::~FileBuilder()
{
    if (m_fout.is_open())
        m_fout.close();
}

bool FileBuilder::has_next() const 
{
    if (m_pkg_queue.empty()) {
        return false;
    }
    return m_pkg_queue.top().get_number() - m_last_writed_pkg_number == 1;
}

void FileBuilder::insert_package(Package&& package) 
{
    assert(package.get_marker() == m_marker);
    if (package.get_number() < m_last_writed_pkg_number)
        return;
    m_pkg_queue.push(std::move(package));
}

bool file_exists(const std::string& filename)
{
    struct stat info;
    return (stat(filename.c_str(), &info) == 0);
}

std::string new_filename(std::string old_filename, std::string suffix)
{
    auto pos = old_filename.find_last_of(".");
    if (pos != std::string::npos) {
        std::string file_ext = old_filename.substr(pos);
        return old_filename.substr(0, pos) + suffix + file_ext;
    }
    return old_filename + suffix;
}

void FileBuilder::initialize_file(const std::string& filename) {
    if (filename.empty())
        throw std::runtime_error("FileBuilder error: empty file name");
    int copy_number = 0;
    std::string tmp_filename = filename;
    while (file_exists(tmp_filename))
    {   
        tmp_filename = new_filename(filename, std::string(".other_") + std::to_string(++copy_number));
    }
    m_filename = tmp_filename;
    m_fout.open(filename + downloadble_file_ext, std::ios::out | std::ios::binary);
    if (!m_fout.is_open())
        throw std::runtime_error(std::string("could not open file: ") + m_filename + downloadble_file_ext);
}

std::string FileBuilder::get_file_name() const
{
    return m_filename;
}

int Server::complete()
{
    std::string old_file_name = m_dir "/" 
    if (rename())
}

void FileBuilder::try_writing() {
    if (m_pkg_queue.empty())
        return;
    while(has_next() && !m_file_is_ready) {
        Package pkg = m_pkg_queue.top();
        if (pkg.get_number() == 1) {
            std::stringstream file_name;
            file_name << m_dir << "/" << pkg.get_data();
            initialize_file(file_name.str());
            m_file_name_is_ready = true;
        } else {
            m_fout.write(pkg.get_data(), pkg.get_data_size());
            if (pkg.get_package_flag() == FLAG_LAST_PACKAGE) {
                m_file_is_ready = true;
            } 
        }
        m_pkg_queue.pop();
        ++m_last_writed_pkg_number;
        m_last_writing_package_time = std::chrono::system_clock::now();
    }
}

bool FileBuilder::file_name_is_ready() const
{
    return m_file_name_is_ready;
}

bool FileBuilder::file_is_ready() const 
{
    return m_file_is_ready;
}

time_point<system_clock>  FileBuilder::get_last_writing_package_time() const
{
    return m_last_writing_package_time;
}