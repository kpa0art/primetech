#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>

#include <iomanip>
#include <malloc.h>
#include <iostream>

#define MAX_PACKAGE_SIZE 1400

#define HEADER_NUMBER_SIZE    sizeof(uint32_t)
#define HEADER_NUMBER_OFFSET  0
#define HEADER_MARKER_SIZE    sizeof(uint32_t)
#define HEADER_MARKER_OFFSET  HEADER_NUMBER_OFFSET + HEADER_NUMBER_SIZE
#define HEADER_FLAG_SIZE      sizeof(uint8_t)
#define HEADER_FLAG_OFFSET    HEADER_MARKER_OFFSET + HEADER_MARKER_SIZE

#define HEADER_SIZE   HEADER_NUMBER_SIZE + HEADER_MARKER_SIZE + HEADER_FLAG_SIZE
#define DATA_OFFSET   HEADER_SIZE
#define MAX_DATA_SIZE MAX_PACKAGE_SIZE - HEADER_NUMBER_SIZE - HEADER_MARKER_SIZE

#define FLAG_LAST_PACKAGE     1
#define FLAG_NOT_LAST_PACKAGE 0

class Package {
public:
    Package();

    Package(const char *package, uint32_t size);

    Package(const Package& other);

    Package(Package&& other);

    ~Package();

    void set_number(uint32_t number);

    void set_marker(uint32_t marker);

    void set_data(const char *data, uint32_t size);

    void set_package_flag(uint8_t flag);

    uint32_t get_number() const;

    uint32_t get_marker() const;

    const char *get_data() const;

    uint32_t get_data_size() const;

    uint8_t get_package_flag() const;

    const char* as_bytes() const;

    uint32_t package_size() const;

    void load_package(const char *package, uint32_t size);

    bool operator<(const Package& other) const;

    Package& operator=(const Package& other);

    bool valid() const;

private:
    char     *m_package;
    uint32_t *m_number;
    uint32_t *m_marker;
    uint8_t  *m_flag;
    char     *m_data;
    int      m_data_size;
    
    void initialize(uint32_t package_size = MAX_PACKAGE_SIZE);
};

void print_headers_as_row();

void print_package_as_row(Package &pkg);