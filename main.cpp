#include <iostream>
#include "Socket.hpp"
#include "Mom.hpp"

/**
 * Function That Runs When The Program Starts
 * @param [argc] - The amount of program arguments
 * @param [argv] - The program arguments
 * @return The program termination status code
 */
int main(int argc, char* argv[]) {

    // Gets The Arguments Needed To Play Musical Chairs//
    int numberOfKids = stoi(argv[1]);
    int numberOfChairs = numberOfKids - 1;

    // Creates The Socket//
    Socket server(PORT, numberOfKids);
    cout << server << endl;

    // Mom Starts Welcoming The Children//
    server.pollWelcome();

    // Mom Gets All Of The Children Situated//
    pollfd* children;
    children = new pollfd[numberOfKids];
    for (int index = 0; index < numberOfKids; index++) {
        children[index] = server.getWorkerSocket()[index];
    }

    // Starts The Musical Chairs Game//
    Mom mom(numberOfChairs, children, server);
    mom.run();

    // Ends Program//
    return 0;
}
