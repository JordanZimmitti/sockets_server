#pragma once
#include <string>
#include <sys/poll.h>
#include "Socket.hpp"
using namespace std;

/**
 * Player Structure For Wrapping The Client Sockets
 */
struct Player {
    bool isAlive = false;
    string name  = "";
    FILE* kidToMom;
    FILE* momToKid;
};

/**
 * Class Mom For Handling The Children For Musical Chairs
 */
class Mom {

private:
    int  numberOfChairs   = 0;
    int  numberOfMarchers = 0;
    int  playersStanding  = 0;
    int  totalPlayers;
    int*    chairs;
    Player* players;
    pollfd* children;
    Socket  server;

    // Normal Private Functions//
    bool checkSockets(map<int, int> requestedChairs);
    void emptyChairs();
    void initRound();
    void playARound();
    void stopTheMusic();

public:

    // Constructor//
    explicit Mom(int numberOfChairs, pollfd* children, const Socket& server);
    ~Mom();

    // Normal Public Functions//
    void run();
};