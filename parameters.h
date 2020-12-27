#ifndef CLIENT_PARAMETERS_H
#define CLIENT_PARAMETERS_H

#include <string>

using namespace std;

struct Parameters {
    string host, port;
    string file_path;
    bool parsed;

    Parameters(int argc, char *argv[]);
};

#endif