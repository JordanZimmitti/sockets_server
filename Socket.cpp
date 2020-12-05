#include "Socket.hpp"
#include "Mom.hpp"

/***********************************************************************************************/

/**
 * Empty Constructor
 */
Socket::Socket() {}

/**
 * Primary Constructor For The Socket
 * @param [port]            - The socket port number
 * @param [numberOfClients] - The max number of clients that are allowed to connect
 */
Socket::Socket(int port, int numberOfClients) {
    this->clientsAllowed = numberOfClients;

    // Creates An Array To Store The Clients That Connect//
    clients = new pollfd[clientsAllowed + 1];
    welcomeSocket = &clients[0];
    workerSocket  = &clients[1];

    // Creates The Socket//
    fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (fileDescriptor < 0) {
        fatal("Socket: Can't create socket");
    }

    // Sets The Socket Info//
    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    info.sin_addr.s_addr = INADDR_ANY;

    // Gets The Hostname//
    gethostname(host, 256);

    // Starts Listening On The Socket//
    cout << "Opened socket: [" << fileDescriptor << "] on port: [" << port << "] for stream i/o" << endl;
    listen();
}

/**
 * Function That Starts Listening On The Socket
 */
void Socket::listen() {

    // Assigns A Name To An Unnamed Socket//
    int bindstatus = ::bind(fileDescriptor, (sockaddr*)&info, sizeof(info));
    if (bindstatus < 0) {
        cout << "Socket: Can't bind socket: " << fileDescriptor << endl;
        exit(1);
    }

    // Start Listening On The Socket//
    bindstatus = ::listen(fileDescriptor, 10);
    if (bindstatus < 0) {
        cout << "Socket: Unable to listen on socket: " << fileDescriptor << endl;
        exit(1);
    }
}

/***********************************************************************************************/

/**
 * Function That Polls For New Clients
 */
void Socket::pollWelcome() {

    // Sets The Welcome Socket//
    *welcomeSocket = {fileDescriptor, POLLIN, 0};
    for (;;) {

        // Starts Looking For The Clients//
        status = ::poll(clients, clientsAllowed + 1, -1);

        // When There Is An Error//
        if (status < 0)  fatal("Socket: Error in poll()");
        if (status == 0) cout << "Poll timed out" << endl;

        // When The Welcome Socket Has A Message From The Client//
        if (welcomeSocket->revents != 0 ) {

            // When There Is An Error With The Welcome Socket//
            if (!(welcomeSocket->revents & POLLIN)){
                cout << "Error involving welcome Socket: " << welcomeSocket->revents << endl;
                exit(1);
            }

            // Welcome The New Client If Possible//
            if (clientsConnected < clientsAllowed) welcome();
        }

        // Disables The Welcome Socket When All The Clients Are Connected//
        welcomeSocket->events = (clientsConnected < clientsAllowed) ? POLLIN : 0;

        // When All Of The Clients Are Connected//
        if (clientsConnected == clientsAllowed) {
            close(welcomeSocket->fd);
            break;
        }
    }
}

/**
 * Function That Welcomes In A Client
 */
void Socket::welcome() {

    // Accepts The New Client//
    int client = accept();
    if (client == 0) {
        cout << "Socket: The connection attempt was rejected" << endl;
        exit(1);
    }

    // Sends The Greeting To The Client//
    FILE* welcome = fdopen(client, "w");
    fprintf(welcome, "%5s", "HELLO");
    fprintf(welcome, "%72s", HELLO);
    fflush(welcome);
}

/**
 * Function That Accepts A New Client Socket And Installs A New Entry For The Polling Table
 * @return The client socket
 */
int Socket::accept() {

    // Define An Initialize Socket Variables//
    sockaddr_in socket {};
    unsigned socketLength = sizeof socket;

    // Accepts A New Client Socket//
    int newFileDescriptor = ::accept(fileDescriptor, (sockaddr*)&socket, &socketLength);
    if (newFileDescriptor < 0) {
        cout << "Socket: Connection was rejected" << endl;
        return 0;
    }

    // Adds The New Client To The Worker Socket//
    pollfd newClient = {newFileDescriptor, POLLIN, 0};
    workerSocket[clientsConnected++] = newClient;
    return newFileDescriptor;
}

/***********************************************************************************************/

/**
 * Function That Polls The Worker Sockets For A Message From The Clients
 * @param [players]         - The players in musical chairs
 * @param [playersStanding] - The players that are still looking for chairs
 * @return A map of what players picked what chairs
 */
map<int, int> Socket::pollWorker(Player* players, int playersStanding) {

    // Creates A Map For The Picked Chairs//
    map<int, int> chairs = map<int, int>();
    for (;;) {

        // Starts Looking For The Clients//
        status = ::poll(clients, clientsAllowed + 1, -1);

        // When There Is An Error//
        if (status < 0)  fatal("Socket: Error in poll");
        if (status == 0) cout << "Poll timed out" << endl;

        // When A Worker Socket Has A Message From One Of The Clients//
        for (int index = 0; index < clientsAllowed; index++) {

            // Gets The Player Associated With The Socket//
            Player* player = &players[index];
            if (!player->isAlive) continue;

            // When The Client Has A Message To Process//
            if (workerSocket[index].revents != 0) {

                // When There Is An Error With The Client Worker Socket//
                if (!(workerSocket[index].revents & POLLIN)) {
                    cout << "Error involving worker Socket: " << workerSocket[index].revents << endl;
                    exit(1);
                }

                // Adds The Players Attempted Chair To The Map//
                int attemptedChair = service(&workerSocket[index]);
                chairs[index] = attemptedChair;
            }
        }

        // Returns The Player Numbers With Their Picked Chair//
        if (chairs.size() == playersStanding) return chairs;
    }
}

/**
 * Function That Services The Client Worker Sockets For An Attempted Chair
 * @param [pollfd] - The client
 * @return The chair fixed by the client
 */
int Socket::service(pollfd* pollfd) {

    // Gets The Message From The Client//
    FILE* file = fdopen(pollfd->fd, "r");
    char name[BUFSIZ];
    char output[BUFSIZ];
    int  attemptedChair = 0;
    fscanf(file, "%1024[^\n] %1024[^\n] %i", name, output, &attemptedChair);

    // Shows The Clients Message//
    cout << name << " " << output << ": " << attemptedChair << endl;
    return attemptedChair;
}

/***********************************************************************************************/

/**
 * Function That Gets The Worker Sockets
 * @return The worker sockets
 */
pollfd *Socket::getWorkerSocket() {
    return workerSocket;
}

/**
 * Prints The Socket Info
 * @param [out] - The output stream
 * @return The socket info
 */
ostream &Socket::print(ostream& out) {
    out << "{" << endl;
    out << "  sin_addr.s_addr = " << inet_ntoa(info.sin_addr) << endl;
    out << "  sin_port = " << ntohs(info.sin_port) << endl;
    out << "}" << endl;
    return out;
}

/***********************************************************************************************/