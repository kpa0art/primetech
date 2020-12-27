#include <string>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <cassert>

#include "parameters.h"

using namespace std;

static void print_help(string program_name) {
    cout << "Use: " << program_name <<  " --host (host_ip) --port (port_number) --file (file_path)\n"
        "   --host - server address\n"
        "   --port - server port\n"
        "   --file - file name to be sent\n"
        "   --help(-h) - print information about flags\n";
}

Parameters::Parameters(int argc, char *argv[]) : parsed(false) {
    assert( "invalid number of arguments" && argc > 0);
    
    char opt_string[] = "";

    enum flags {
        HOST,
        PORT,
        FILE,
        HELP
    };

    struct option long_opts[] = {
        { "host", required_argument, 0, 0 },
        { "port", required_argument, 0, 0 },
        { "file", required_argument, 0, 0 },
        { "help", no_argument,       0, 0 },
        {      0,                 0, 0, 0 }
    };

    int opt;
    int option_index = 0;

    while ( (opt = getopt_long( argc, argv, opt_string, long_opts, &option_index )) != -1 ) {
        switch ( opt ) 
        {
        case 0:
            switch ( option_index ) 
            {
            case HOST:
                (*this).host = string(optarg);
                break;

            case PORT:
                (*this).port = string(optarg);
                break;

            case FILE:
                (*this).file_path = string(optarg);
                break;

            case HELP:
                print_help(argv[0]); 
                break;

            default:
                cout << "here323" << endl;
                break;
            }
            break;

        case 'h':
        case '?':
            print_help(argv[0]);
            break;
        
        default:
            break;
        }
    }    
}

int main(int argc, char *argv[]) {

    Parameters params(argc, argv);
    
    if ( params.parsed ) {
        cout << "Host: " << params.host << " Port: " << params.port << " File: " << params.file_path << endl;
    }
    
    return 0;
}
