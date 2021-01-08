#include "package.h"


/** \brief Конструктор пакета
 *  
 * Функция создает объект пакета.
 * 
 * \exception runtime_error
 * В процесе инициализации может произойти случай, когда память под объект 
 * не будет выделена. В таком случае вызывается исключение runtime_error.
 */
Package::Package()
{
    initialize();
}

/** \brief Конструктор пакета
 *  
 * Функция создает объект пакета принимая другой \p package , представленный набором 
 * байтов. 
 * 
 * \exception runtime_error
 * В процесе инициализации может произойти случай, когда память под объект 
 * не будет выделена. В таком случае вызывается исключение runtime_error.
 * 
 * \param[in] package    Пакет, упакованный в массив байтов.
 * \param[in] size       Размерпакета в байтах. 
 */
Package::Package(const char *package, uint32_t size)
{
    initialize(size);
    load_package(package, size);
}

/** \brief Деструктор пакета
 *  
 * Функция освобождает ресурсы, выделенные при создании объекта пакета. 
 */ 
Package::~Package()
{
    if (m_package != nullptr)
        free(m_package);
}

/** \brief Конструктор пакета 
 * 
 * Функция создает объекта пакета, при этом объект \p other пакета  передает
 * свои ресурсы. После этого \p other становится невалидным.
 *   
 * \param[in] other    Другой объект пакета.
 */ 
Package::Package(Package&& other)
    : m_package(other.m_package)
    , m_number(other.m_number)
    , m_marker(other.m_marker)
    , m_flag(other.m_flag)
    , m_data(other.m_data)
    , m_data_size(other.m_data_size)
{
    other.m_package = nullptr;
    other.m_number = nullptr;
    other.m_marker = nullptr;
    other.m_flag = nullptr;
    other.m_data = nullptr;
    other.m_data_size = 0;
}

/** \brief Конструктор пакета 
 * 
 * Функция создает объекта пакета, при этом проискодит копирование ресурсов
 * \p other пакета.
 * 
 * \exception runtime_error
 * В процесе инициализации может произойти случай, когда память под объект 
 * не будет выделена. В таком случае вызывается исключение runtime_error.
 *   
 * \param[in] other    Другой объект пакета.
 */ 
Package::Package(const Package& other)
{
    initialize(other.package_size());
    memcpy(m_package, other.m_package, other.package_size());
}

/** \brief Установка номера пакета
 * 
 * Функция устанавливает номер пакета, использующийся для определения
 * место пакета в передаваемом потоке пакетов. 32-битный тип дает 
 * возможность сумарно передать данные пакетами размером до 
 * 2^32 * MAX_DATA_SIZE байтов.  
 * 
 * \warning 
 * В случае передачи ресурсов по средством std::move() для функций, требующих
 * rvalue ссылку, объект пакета становится невалидным, поэтому при попытке
 * установить номер пакета пройзойдет assert().
 * 
 * \param[in] number    Номер пакета.
 */ 
void Package::set_number(uint32_t number)
{
    assert(m_package != nullptr);
    *m_number = number;
}

/** \brief Установка идентификатора пакета
 * 
 * Функция устанавливает идентификатора пакета, использующийся для определения
 * пакета к потоку передаваемых пакетов. В случае совпадения идентификаторов 
 * потоков ситуация неопределена. Рекомендуется использовать случайное число
 * с большим периодом, либо хеш-сумма файла, умещающаяся в 4 байтный формат.
 * 
 * \warning 
 * В случае передачи ресурсов по средством std::move() для функций, требующих
 * rvalue ссылку, объект пакета становится невалидным, поэтому при попытке
 * установить номер пакета пройзойдет assert().
 * 
 * \param[in] marker    Номер пакета.
 */ 
void Package::set_marker(uint32_t marker)
{
    assert(m_package != nullptr);
    *m_marker = marker;
}

/** \brief Запись данных в пакет
 * 
 * Функция записывает \p data в пакет в виде массива байтов размером \p size .
 * 
 * \warning 
 * В случае передачи ресурсов по средством std::move() для функций, требующих
 * rvalue ссылку, объект пакета становится невалидным, поэтому при попытке
 * установить номер пакета пройзойдет assert(). Так же в случае, если размер
 * данных превысит MAX_DATA_SIZE, будет вызван assert().
 * 
 * \param[in] data    Данные представляющие собой массив байтов.
 * \param[in] size    Размер массива.
 */ 
void Package::set_data(const char *data, uint32_t size)
{
    assert(m_package != nullptr);
    assert(size <= MAX_DATA_SIZE);
    memcpy(m_data, data, size);
    m_data_size = size;
}

/** \brief Установка флага пакета.
 * 
 * Функция устанавливает флаг пакета. Существует два флага пакета:
 * FLAG_LAST_PACKAGE и FLAG_NOT_LAST_PACKAGE. Они обозначают, является ли
 * текущий пакет в потоке пакетов последним. 
 * 
 * \warning 
 * В случае передачи ресурсов по средством std::move() для функций, требующих
 * rvalue ссылку, объект пакета становится невалидным, поэтому при попытке
 * установить номер пакета пройзойдет assert(). Так же в случае утсановки
 * неизвествного флага, будет вызван assert(). 
 * 
 * \param[in] flag    Флаг пакета.
 */ 
void Package::set_package_flag(uint8_t flag)
{
    assert(m_package != nullptr);
    assert(flag == FLAG_LAST_PACKAGE || flag == FLAG_NOT_LAST_PACKAGE);
    *m_flag = flag;
}

/** \brief Установка флага пакета.
 * 
 * Функция устанавливает флаг пакета. Существует два флага пакета:
 * FLAG_LAST_PACKAGE и FLAG_NOT_LAST_PACKAGE. Они обозначают, что является ли
 * текущий пакет в потоке пакетов последним. 
 * 
 * \warning 
 * В случае передачи ресурсов по средством std::move() для функций, требующих
 * rvalue ссылку, объект пакета становится невалидным, поэтому при попытке
 * установить номер пакета пройзойдет assert(). Так же в случае утсановки
 * неизвествного флага, будет вызван assert(). 
 * 
 * \param[in] flag    Флаг пакета.
 */ 
uint32_t Package::get_number() const
{
    assert(m_package != nullptr);
    return *m_number;
}

/** \brief Вернуть идентификатор пакета
 * 
 * Функция возвращает копию идентификатор пакета, определяющего к какому потоку 
 * пакетов относится данный пакет. Идентификатор может быть изменен с помощью
 * функции set_marker();
 * 
 * \return Возвращает индетификатор пакета в виде 4 байтного целого числа.
 */ 
uint32_t Package::get_marker() const
{
    assert(m_package != nullptr);
    return *m_marker;
}

/** \brief Вернуть данные пакета.
 * 
 * Функция возвращает указатель на данные, передаваемые в пакете. Данные
 * представляют собой массив байтов. Размер данных в пакете можно узнать,
 * вызвав метод get_data_size(). 
 * 
 * \warning
 * Функция возращает указатель на содержимое пакета. Содержимое можно
 * модифицировать, поэтому желательно скопировать данные с использованием
 * memcpy() в некоторый буфер, умещающий данные пакета.
 * 
 * \return  Возвращает указатель на массив байтов.
 */ 
const char *Package::get_data() const
{
    assert(m_package != nullptr);
    return m_data;
}

/** \brief Вернуть размер данные пакета.
 * 
 * Функция возвращает размер данных, считающийся в количестве байтов.
 *
 * \return  Возвращает число байтов данных.
 */ 
uint32_t Package::get_data_size() const
{
    assert(m_package != nullptr);
    return m_data_size;
}

/** \brief Вернуть флаг пакета.
 * 
 * Функция возвращает флаг пакета. Пакет длжен иметь флаги 
 *
 * \return  Возвращает число байтов данных.
 */ 
uint8_t Package::get_package_flag() const
{
    assert(m_package != nullptr);
    assert(*m_flag == FLAG_NOT_LAST_PACKAGE || 
           *m_flag == FLAG_LAST_PACKAGE);
    return *m_flag;
}

/** \brief Вернуть пакет как массив байтов.
 * 
 * Функция возвращает пакет, упакованный в массив байтов.
 * 
 * \warning
 * Функция возращает указатель массив байтов, в который упакован пакет.
 * Содержимое можно модифицировать, поэтому желательно скопировать данные
 * с использованием memcpy() в некоторый буфер, умещающий весь пакет.
 *
 * \return  Возвращает указатель на массив байтов, в который упакован пакет.
 */ 
const char *Package::as_bytes() const
{
    return m_package;
}

/** \brief Вернуть размер пакета в байтах.
 * 
 * Функция возвращает размер пакета в байтах.
 *
 * \return  Возвращает указатель на массив байтов, в который упакован пакет.
 */ 
uint32_t Package::package_size() const
{
    if (m_package == nullptr)
        return 0;
    return HEADER_SIZE + m_data_size;
}


/** \brief Загрузить пакет, упакованный в массив байтов.
 * 
 * Функция принимает пакет, упакованный в массив байтов. В случае, если объект 
 * пакета передал ресурсы другому объекту пакету, то текущий пакет
 * проинициализирует себ, чтобы скопировать данные \p package . 
 *
 * \param[in] package    Указатель на массив из байтов упакованного пакет.
 * \param[in] size       Размер пакета.
 */ 
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

/** \brief Сравнение пакетов
 * 
 * Функция  возращает булевское значение. Пакет считается меньше в том случае,
 * порядновый номер пакета меньше по сравнению с другим.
 *
 * \param[in] other     Ссыдкана объект пакета.
 * 
 * \return true, если номер пакета больше другого, и false иначе.
 */ 
bool Package::operator<(const Package &other) const
{
    return (*this).get_number() > other.get_number();
}

/** \brief Копирование пакета
 * 
 * Функция  возращает ссылку на созданный новый объект пакета, в который 
 * скопированы данные ресурсы друго объекта пакета.
 *
 * \param[in] other     Ссылка на объект пакета.
 * 
 * \return Ссылка на скопированный объект пакета.
 */ 
Package& Package::operator=(const Package& other)
{
    assert(other.package_size() >= HEADER_SIZE);
    initialize(other.package_size());
    memcpy(m_package, other.m_package, other.package_size());
    m_data_size = other.m_data_size;
    return *this;
}

/** \brief Инициализация объекта
 * 
 * Функция  инициализирует объект пакета, резервируя память под ресурсы. 
 * Параметр \p package_size не должен превышать MAX_PACKAGE_SIZE. 
 * 
 * \exception runtime_error
 * В процесе инициализации может произойти случай, когда память под ресурсы 
 * не будет выделена. В таком случае вызывается исключение runtime_error.
 *
 * \param[in] package_size     Теоретичесикй размер пакета.
 */ 
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


/** \brief Валидация пакета
 * 
 * Функция осуществляет простую валидацию пакета. Ее полезно использовать,
 * когда вызыввается конструктор объекта пакета, либо метод load_package(). 
 * 
 * \return true, если пакет считается валидным, falseиначе
 */ 
bool Package::valid() const
{
    return m_package != nullptr && package_size() >= HEADER_SIZE;
}

#ifdef DEBUG

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

#endif