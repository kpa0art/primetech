#ifndef PACKAGE_H
#define PACKAGE_H

#include <cstdint>
#include <cstring>
#include <cassert>

#include <malloc.h>

#define MAX_PACKAGE_SIZE 1400


#define HEADER_NUMBER_SIZE    sizeof(uint32_t)
#define HEADER_NUMBER_OFFSET  0
#define HEADER_MARKER_SIZE    sizeof(uint32_t)
#define HEADER_MARKER_OFFSET  HEADER_NUMBER_SIZE

#define HEADER_SIZE   uint32_t(HEADER_NUMBER_SIZE + HEADER_MARKER_SIZE)
#define DATA_OFFSET   HEADER_SIZE
#define MAX_DATA_SIZE uint32_t(MAX_PACKAGE_SIZE - HEADER_NUMBER_SIZE - HEADER_MARKER_SIZE)

class Package {
public:
    Package() {
        initialize();
    }

    Package(const char *package, uint32_t size) {
        initialize();
        load_package(package, size);    
    }

    ~Package() {
        free(m_package);
    }

    void set_number(uint32_t number) {
        *m_number = number; 
    }

    void set_marker(uint32_t marker) {
        *m_marker = marker;
    }

    void set_data(const char *data, uint32_t size) {
        assert(size <= MAX_DATA_SIZE);
        memcpy(m_data, data, size);
        m_data_size = size;
    }

    uint32_t get_number() const {
        return *m_number;
    }

    uint32_t get_marker() const {
        return *m_marker;
    }

    const char *get_data() const {
        return m_data;
    }

    uint32_t get_data_size() const {
        return m_data_size;
    }

    const char* as_bytes() const {
        return m_package;
    }

    uint32_t package_size() const {
        return HEADER_SIZE + m_data_size;
    }

    void load_package(const char *package, uint32_t size) {
        assert(size <= MAX_PACKAGE_SIZE);
        assert(size >= HEADER_SIZE);
        memcpy(m_package, package, size);
        uint32_t data_size = size - HEADER_SIZE;
        m_data_size = (data_size > 0) ? data_size : 0;
    }

    bool operator<(const Package& other)
    {
        return (*this).get_number() > other.get_number();
    }

    void info() {
        std::cout << "CURR_PKG_SIZE: " << package_size() << std::endl;
        std::cout << " Number: " << get_number() << " OFFSET: " << HEADER_NUMBER_OFFSET << " SIZE: " << HEADER_NUMBER_SIZE << std::endl;
        std::cout << " Marker: " << get_marker() << " OFFSET: " << HEADER_MARKER_OFFSET << " SIZE: " << HEADER_MARKER_SIZE << std::endl;
        std::cout << " Body: " << get_data() << " OFFSET: " << DATA_OFFSET << " SIZE: " << get_data_size() << std::endl;
        std::cout << " REAL PKG SIZE: " << malloc_usable_size(m_package) << std::endl;
    }

private:
    uint32_t *m_number;
    uint32_t *m_marker;
    char     *m_data;
    int      m_data_size;
    char     *m_package;

    void initialize() {
        m_package = (char *)calloc(1, MAX_PACKAGE_SIZE);
        if (m_package == NULL) {
            throw std::runtime_error("could not allocate memmory for package");
        }
        m_number = (uint32_t*)(m_package + HEADER_NUMBER_OFFSET);
        m_marker = (uint32_t*)(m_package + HEADER_MARKER_OFFSET);
        m_data   = (char*)(m_package + DATA_OFFSET);
        m_data_size  = 0;    
    }
};

#endif