#include "package.h"

Package::Package()
{
    initialize();
}

Package::Package(const char *package, uint32_t size)
{
    initialize(size);
    load_package(package, size);
}

Package::~Package()
{
    if (m_package != nullptr)
        free(m_package);
}

Package::Package(Package&& package)
    : m_package(package.m_package)
    , m_number(package.m_number)
    , m_marker(package.m_marker)
    , m_flag(package.m_flag)
    , m_data(package.m_data)
    , m_data_size(package.m_data_size)
{
    package.m_package = nullptr;
    package.m_number = nullptr;
    package.m_marker = nullptr;
    package.m_flag = nullptr;
    package.m_data = nullptr;
    package.m_data_size = 0;
}

Package::Package(const Package& other)
{
    initialize(other.package_size());
    memcpy(m_package, other.m_package, other.package_size());
}

void Package::set_number(uint32_t number)
{
    assert(m_package != nullptr);
    *m_number = number;
}

void Package::set_marker(uint32_t marker)
{
    assert(m_package != nullptr);
    *m_marker = marker;
}

void Package::set_data(const char *data, uint32_t size)
{
    assert(m_package != nullptr);
    assert(size <= MAX_DATA_SIZE);
    memcpy(m_data, data, size);
    m_data_size = size;
}

void Package::set_package_flag(uint8_t flag)
{
    assert(m_package != nullptr);
    *m_flag = flag;
}

uint32_t Package::get_number() const
{
    assert(m_package != nullptr);
    return *m_number;
}

uint32_t Package::get_marker() const
{
    assert(m_package != nullptr);
    return *m_marker;
}

const char *Package::get_data() const
{
    assert(m_package != nullptr);
    return m_data;
}

uint32_t Package::get_data_size() const
{
    assert(m_package != nullptr);
    return m_data_size;
}

uint8_t Package::get_package_flag() const
{
    assert(m_package != nullptr);
    return *m_flag;
}

const char *Package::as_bytes() const
{
    return m_package;
}

uint32_t Package::package_size() const
{
    if (m_package == nullptr)
        return 0;
    return HEADER_SIZE + m_data_size;
}

void Package::load_package(const char *package, uint32_t size)
{
    assert(size <= MAX_PACKAGE_SIZE);
    assert(size >= HEADER_SIZE);
    if (m_package == nullptr)
        initialize(size);
    memcpy(m_package, package, size);
    uint32_t data_size = size - uint32_t(HEADER_SIZE);
    m_data_size = (data_size > 0) ? data_size : 0;
}

bool Package::operator<(const Package &other) const
{
    return (*this).get_number() > other.get_number();
}

Package& Package::operator=(const Package& other)
{
    assert(other.package_size() >= HEADER_SIZE);
    initialize(other.package_size());
    memcpy(m_package, other.m_package, other.package_size());
    m_data_size = other.m_data_size;
    return *this;
}

void Package::initialize(uint32_t package_size)
{
    m_package = (char *)calloc(1, package_size);
    if (m_package == NULL)
    {
        throw std::runtime_error("could not allocate memmory for package");
    }
    m_number = (uint32_t *)(m_package + HEADER_NUMBER_OFFSET);
    m_marker = (uint32_t *)(m_package + HEADER_MARKER_OFFSET);
    m_flag = (uint8_t *)(m_package + HEADER_FLAG_OFFSET);
    m_data = (char *)(m_package + DATA_OFFSET);
    m_data_size = 0;
}

bool Package::valid() const
{
    return package_size() >= HEADER_SIZE;
}

void print_headers_as_row()
{
    std::cout << std::setw(10) << "No" << " "
        << std::setw(10) << "Marker" << " "
        << std::setw(8) << "End_flag" << " "
        << std::setw(5) << "HSize" << " "
        << std::setw(5) << "DSize" << " "
        << std::setw(5) << "PSize" << " "
        << std::setw(5) << "Mem" << std::endl;
    return ;
}

void print_package_as_row(Package &pkg)
{
    int mem_size = malloc_usable_size((void *)pkg.as_bytes());
    const char *flag = { pkg.get_package_flag() == FLAG_LAST_PACKAGE ? "YES" : "NO"};
    std::cout << std::right << std::setw(10) << std::to_string(pkg.get_number()) << " "
        << std::setw(10) << std::to_string(pkg.get_marker()) << " "
        << std::setw(8) << flag << " "
        << std::setw(5) << HEADER_SIZE << " "
        << std::setw(5) << pkg.get_data_size() << " "
        << std::setw(5) << pkg.package_size() << " "
        << std::setw(5) << mem_size << std::endl;
    return ;
}