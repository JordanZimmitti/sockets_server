#include "Mom.hpp"

/***********************************************************************************************/

/**
 * Primary Constructor For Mom
 * @param [numberOfChairs] - The number of chairs needed for the game
 * @param [children]       - The children participating in musical chairs
 * @param [server]         - The Server for communicating with the clients
 */
Mom::Mom(int numberOfChairs, pollfd* children, const Socket& server) {
    this->numberOfChairs   = numberOfChairs;
    this->numberOfMarchers = numberOfChairs + 1;
    this->totalPlayers     = numberOfChairs + 1;
    this->chairs           = new int[numberOfChairs];
    this->players          = new Player[numberOfChairs + 1];
    this->children         = children;
    this->server           = server;
    srand(time(nullptr));
}

/**
 * Destructor For Mom
 */
Mom::~Mom() {
    delete [] chairs;
    delete [] players;
}

/***********************************************************************************************/

/**
 * Function That Removes Any Kid That May Be Sitting In A Chair
 */
void Mom::emptyChairs() {

    // Makes Sure No Kid Is Sitting In A Chair//
    for (int index = 0; index <= numberOfChairs; index++) {
        chairs[index] = index == numberOfChairs ? '\0' : -1;
    }
}

/**
 * Function That Starts The Musical Chairs Game
 */
void Mom::run() {

    // Shows That The Children Can Communicate With Mom By Giving Her Their Names//
    cout << "The children say hi and tell mom their names and that they are ready to play" << endl;

    // Instantiates The Players From The Children//
    int index = 0;
    while (index < totalPlayers) {
        pollfd* child  = &children[index];
        Player* player = &players[index];

        // Opens The Input And Output Streams//
        player->momToKid = fdopen(child->fd, "w");
        player->kidToMom = fdopen(child->fd, "r");
        if (player->momToKid == nullptr || player->kidToMom == nullptr) continue;

        // Gets The Output And Name Of The Client//
        char output[BUFSIZ];
        char name[BUFSIZ];
        fscanf(player->kidToMom, "%1024[^\n] %1024[^\n]", output, name);

        // Sets The Name//
        player->name    = name;
        player->isAlive = true;

        // Shows Mom That The Child Is Ready To Play Musical Chairs//
        cout << output << name << endl;
        index++;
    }

    // Starts Musical Chairs//
    int round = 1;
    for (;;) {
        cout << "\nRound " << round << endl;
        cout << "-----------------------------------------------------" << endl;
        playARound();
        cout << "-----------------------------------------------------" << endl;
        if (numberOfChairs == 1) return;
        numberOfChairs--;
        numberOfMarchers--;
        round++;
    }
}

/**
 * Function That Plays A Single Round Of Musical Chairs
 */
void Mom::playARound() {
    initRound();
    stopTheMusic();
}

/**
 * Prepares The Round Of Musical Chairs
 */
void Mom::initRound() {

    // Empties The Chairs//
    cout << "Mom makes sure all of the chairs are empty" << endl;
    emptyChairs();

    // Sends The 'GETUP' Command To The Players And How Many Chairs Are In Play//
    for (int index = 0; index < totalPlayers; index++) {
        Player* player = &players[index];
        if (!player->isAlive) continue;
        fprintf(player->momToKid, "%5s", "GETUP");
        fprintf(player->momToKid, "%60s\n%i\n", GETUP, numberOfChairs);
        fflush(player->momToKid);
    }
    cout << "Mom tells the players to gather around the chairs and start marching" << endl;
}

/***********************************************************************************************/

/**
 * Function That Stops The Music
 */
void Mom::stopTheMusic() {

    // Plays The Music While The Kids March//
    int musicTimer = rand() % 4 + 1;
    sleep(musicTimer);

    // Shows How Long Mom Left The Music Going On For//
    cout << "\nMom plays the music for [" << musicTimer << "] seconds then stops the music" << endl;

    // Sends The 'SIT' Command To The Players//
    for (int index = 0; index < totalPlayers; index++) {
        Player* player = &players[index];
        if (!player->isAlive) continue;
        fprintf(player->momToKid, "%5s", "SIT__");
        fprintf(player->momToKid, "%50s", SIT);
        fflush(player->momToKid);
    }

    // The Players Attempt To Pick A Chair//
    playersStanding = numberOfMarchers;
    for(;;) {

        // Gets The Requested Chairs From The Clients//
        map<int, int> requestedChairs = server.pollWorker(players, playersStanding);
        cout << endl;

        // Each Player Attempts To Sit In Their Requested Chair//
        bool isDone = checkSockets(requestedChairs);
        if (isDone) break;
    }
}

/**
 * Function That Handles How Each Player Attempts To Sit In Their Requested Chair
 * @param [requestedChairs] - A map containing the requested chair and they player who picked it
 * @return Whether every player except for one found a chair to sit in
 */
bool Mom::checkSockets(map<int, int> requestedChairs) {
    bool isDone = false;
    map<int, int>::iterator it;
    for (it = requestedChairs.begin(); it != requestedChairs.end(); it++) {
        int playerNumber = it->first;
        int chair = it->second;
        Player *player = &players[playerNumber];

        // When There Is One Player Standing//
        if (playersStanding == 1) {

            // Sends That The User Lost Musical Chairs//
            player->isAlive = false;
            fprintf(player->momToKid, "%5s", "QUIT_");
            fprintf(player->momToKid, "%68s", QUIT);
            fflush(player->momToKid);

            // Shows That The User Lost Musical Chairs//
            cout << "\n" << player->name << " is the last one standing and lost the game :(" << endl;
            isDone = true;
        }

        // When The Chair A Player Is Attempting To Sit In Is Open//
        else if (chairs[chair] == -1) {

            // Sits The Player In The Chair//
            chairs[chair] = playerNumber;
            playersStanding--;

            // When There Is More Than Two Marchers Left//
            if (numberOfMarchers != 2) {

                // Sends That The Player Is Now Sitting In Their Attempted Chair//
                fprintf(player->momToKid, "%5s", "ACK__");
                fprintf(player->momToKid, "%46s\n%i\n", ACK, chair);
                fflush(player->momToKid);

                // Shows That The Player Is Sitting In Their Attempted Chair//
                cout << player->name << " is sitting in seat: " << chair << endl;
            }

            // When A Player Wins//
            else {

                // Sends That The Player Won Musical Chairs//
                fprintf(player->momToKid, "%5s", "PRIZE");
                fprintf(player->momToKid, "%68s\n", PRIZE);
                fflush(player->momToKid);

                // Shows That The Player Won Musical Chairs//
                cout << player->name << " is sitting in seat: " << chair << endl;
                cout << player->name << " is the winner of musical chairs!!" << endl;
                isDone = true;
            }
        }

        // When A Player Attempts To Sit In An Already Taken Chair//
        else {

            // Gets The List Of Available Chairs//
            stringstream ss;
            for (int index = 0; index < numberOfChairs; ++index) {
                if (chairs[index] == -1) ss << index;
            }

            // Sends The List Of Open Chairs To The Player//
            fprintf(player->momToKid, "%5s", "NACK_");
            fprintf(player->momToKid, "%35s\n%s\n%i\n", NACK, ss.str().c_str(), chair);
            fflush(player->momToKid);

            // Shows That The Player Needed To Pick A New Chair//
            cout << player->name << " needs to pick a new chair because chair [" << chair << "] is taken"<< endl;
        }
    }
    return isDone;
}

/***********************************************************************************************/