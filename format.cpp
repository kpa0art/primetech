#include "format.h"
#include <iomanip>

std::ostream& operator<<(std::ostream& os, std::tm *dt)
{
    return os
        << std::setfill('0') << std::setw(4) << dt->tm_year + 1900 << "-"
        << std::setfill('0') << std::setw(2) << dt->tm_mday << "-"
        << std::setfill('0') << std::setw(2) << dt->tm_mon + 1 << " "
        << std::setfill('0') << std::setw(2) << dt->tm_hour << ":"
        << std::setfill('0') << std::setw(2) << dt->tm_min << ":"
        << std::setfill('0') << std::setw(2) << dt->tm_sec;
}