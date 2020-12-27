#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

#include "parameters.h"

using namespace std;

class Client {

    Client(string address, int port) {
        sockaddr_in address;
        address.sin_family = AF_INET;
        //address.sin

    };

    Send(char *file_path) {

    }

    sendFile() int disodis
    

private:
    int socket_fd;

};

int main(int argc, char *argv[]) {

    Parameters params(argc, argv);
    if ( !params.parsed ) {
        cout << "not parsed" << endl;
    }

    
    Client client( params.host, params.port );


    return 0;
}