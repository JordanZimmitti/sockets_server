#pragma once
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <zconf.h>
#include <poll.h>
#include "shared.h"
#include "tools.hpp"
using namespace std;
struct Player;

/**
 * Class For Handling The Sockets
 */
class Socket {

private:

    // Base Socket Variables//
    char host[256]       {};
    int fileDescriptor = -1;
    sockaddr_in info   = {0};

    // Server Socket Variables//
    int status           = -1;
    int clientsConnected = 0;
    int clientsAllowed   = 0;
    pollfd* clients;
    pollfd* welcomeSocket;
    pollfd* workerSocket;

    // Normal Functions//
    int  accept();
    int  service(pollfd *pollfd);
    void listen();
    void welcome();

public:

    // Constructor//
    explicit Socket();
    explicit Socket(int port, int numberOfClients);

    // Inline Functions//
    pollfd* getWorkerSocket();

    // Normal Functions//
    void pollWelcome();
    map<int, int> pollWorker(Player* players, int playersStanding);
    ostream& print(ostream& out);
};

// Overload '<<' For Cout To Work//
inline ostream& operator <<(ostream& out, Socket &socket) {
    return socket.print(out);
}
