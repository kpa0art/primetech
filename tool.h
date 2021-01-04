#pragma once

#include <string>
#include <sys/stat.h>

bool file_exists(const std::string& filename)
{
    struct stat info;
    return (stat(filename.c_str(), &info) == 0);
}

bool dir_exists(const std::string& dirname)
{
    struct stat info;
    if (stat(dirname.c_str(), &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}