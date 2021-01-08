#include "file_builder.h"
#include "format.h"

#include <cstdio>
#include <sys/stat.h>
#include <sstream>
#include <regex>

// регулярное выражение для валидации  имени файла
const std::regex file_name_regex("^[\\w|\\d|.|&|,|:|;]+$"); 

/** \brief Конструктор файлового сборщика 
 * 
 * Функция инициализирует объект файлового сборщика принимая в качестве 
 * аргументов \p marker директорию, куда сохранить полученный файл.
 * 
 * \note
 * \p marker необходим для внутренней проверки идентификации, чтобы быть 
 * уверенным, что принимающие пакеты принадлежат одному потоку пакетов.
 */ 
FileBuilder::FileBuilder(const std::string& dir, uint32_t marker)
    : m_marker(marker)
    , m_last_writed_pkg_number(0)
    , m_file_name_is_ready(false)
    , m_file_body_is_ready(false)
    , m_file_is_created(false)
    , m_last_writing_package_time(system_clock::now())
{
    m_dir = dir;
    if (m_dir.find_last_of("/") != dir.size() - 1)
        m_dir.append("/");
}

/** \brief Деструктор файлового сборщика 
 * 
 * Функция закрывает открытый файл, и если он не готов, то удаляет его.
 */ 
FileBuilder::~FileBuilder()
{
    if (m_fout.is_open())
    {
        m_fout.close();
        if (!file_is_ready())
            remove(m_tmp_filename.c_str());
    }
}

/** \brief Проверка на присутствие следующего пакета
 * 
 * Функция сравнивает, если ли следующий пакет в очереди или нет.
 * 
 * \return true, если пакет пристствует, false иначе.
 */ 
bool FileBuilder::has_next_package() const 
{
    if (m_pkg_queue.empty()) {
        return false;
    }
    return m_pkg_queue.top().get_number() - m_last_writed_pkg_number == 1;
}

/** \brief Добавление пакета очередь пакетов.
 * 
 * Функция добавляет очередной пакет в очередь пакетов. Так как используется 
 * очередь с приоритетом, то пакет будет автомтически добавлен в нужную 
 * позицию.
 * 
 * \warning
 * Осуществляется проверка, принадлежит ли переданный пакет тому же потоку 
 * пакетов, что ожидает файловый сборщик. В случае неудачи будет вызван assert.
 * 
 * \param[in] package    Очередной пакет из потока пакетов.
 */ 
void FileBuilder::insert_package(Package&& package) 
{
    assert(package.get_marker() == m_marker);
    if (package.get_number() < m_last_writed_pkg_number)
        return;
    m_pkg_queue.push(std::move(package));
}

/** \brief Определено ли имя фала.
 * Функция проверяет, определил ли файловый сборщик имя файла.
 * 
 * \return true, если файл готов, fasle иначе.
 */ 
bool FileBuilder::file_name_is_ready() const 
{
    return m_file_name_is_ready;
}

/** \brief Получитькопию имени файла 
 * 
 * Функция возращает имя фала, если файловому сборщику удалост определить его.
 * 
 * \note
 * Для того, чтобы понять, определил ли файловый сборщик имя, следует вызвать
 * метод file_name_is_ready
 * 
 * \return Возращает имя файла, если файловый сборщик определил его, иначе
 * верентся пустая строка. 
 */ 
std::string FileBuilder::get_file_name() const
{
    if (!file_name_is_ready())
        return std::string();
    return m_origin_filename;
}

/** \brief Готов ли файл 
 * 
 *  Функция проверя, завершил ли сборку файловый сборщик сборку файла.
 *
 * \return true, если вайловый сборщик собрал файл, false иначе.
 */ 
bool FileBuilder::file_is_ready() const 
{
    return m_file_body_is_ready && m_file_name_is_ready;
}

/** \brief Копию время последней записи пакета в во временный файл.
 * 
 * Функция возвращает временную метку, которая была зафиксирована при последней
 * записи пакета файловым сборщиком. 
 * 
 * \return Время (time_point<system_clock>) последней записи пакета. 
 */ 
time_point<system_clock>  FileBuilder::get_last_writing_package_time() const
{
    return m_last_writing_package_time;
}

/** \brief Создание файла 
 * 
 * Функция создает временный файл, в который будет писаться вся информация, 
 * полученная файловым сборщиком.
 * 
 * \note
 * В случае неудачи, соответствующий код ошибки заносится в errno.
 * 
 * \return -1, в случае успеха создания файла, 0 иначе.  
 */ 
// int FileBuilder::create_file()
// {
//     auto ts = std::time(0);
//     std::ostringstream ss;
//     ss << m_dir << "[" << localtime(&ts) << "]-" << m_marker;
//     m_tmp_filename = ss.str();
//     m_fout.open(m_tmp_filename, std::ios::binary|std::ios::out);
//     if (!m_fout.is_open() || !m_fout.good()){
//         return -1;    
//     }
//     return 0;
// }

/** \brief Обработка пакетов.
 * 
 * В этой функции полученные пакеты файловый сборщик записывает последовательно
 * в файл. Так же эта функция берет на себя ответственность по созданию файла.
 * 
 * \note
 * По началу файловый менеджер пишет данные во временный файл имеющий формат:
 * [time_create]-marker. Ошибки, возращаемы функцией следующие: 
 * ErrInvalidFileName - файл с таким именем нельзя созать, 
 * ErrExpectPackage   - не достает пакета для записи,
 * ErrErrno           - код ошибки смотреть в errno.
 * 
 * \warning
 * В случае, если имя файла, полученное файловым сборщиком, уже занятов в текущей 
 * директории, то файловый сборщик попытается удалить его.
 * 
 * \return 0, в случае успеха, значение меньшее 0 иначе.
 */ 
int FileBuilder::process()
{
    // if (!m_file_is_created)
    // {
    //     int result;
    //     if ((result = create_file()) != 0)
    //         return ErrErrno;
    //     m_file_is_created = true;
    // }
    while (has_next_package() && !file_is_ready())
    {
        const Package &package = m_pkg_queue.top();
        if (package.get_number() == 1)
        {
            std::stringstream fresh_file_name;
            fresh_file_name.write(package.get_data(), package.get_data_size());
            fresh_file_name << "\0";
            if (!std::regex_match(fresh_file_name.str(), file_name_regex))
                return ErrInvalidFileName;    
            m_origin_filename = m_dir + fresh_file_name.str();
            m_fout.open(m_origin_filename, std::ios::binary|std::ios::out);
            if (!m_fout.is_open() || !m_fout.good())
                return ErrCouldNotCreateFile;    
            m_file_name_is_ready = true;     
        } else {
            m_fout.write(package.get_data(), package.get_data_size());
            if (package.get_package_flag() == FLAG_LAST_PACKAGE) {
                m_fout.close();
                m_file_body_is_ready = true;
            }  
        }
        m_pkg_queue.pop();
        ++m_last_writed_pkg_number;
        m_last_writing_package_time = std::chrono::system_clock::now();
    }
    if (!file_is_ready())
        return ErrExpectPackage;
    // if (file_exists(m_origin_filename) && 
    //     remove(m_origin_filename.c_str()) != 0)
    //     return ErrErrno;
    // if (rename(m_tmp_filename.c_str(), m_origin_filename.c_str()) != 0)
    //     return ErrErrno;
    return 0;
}
