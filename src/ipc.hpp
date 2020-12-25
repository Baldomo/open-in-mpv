#ifndef MPVIPC_HPP_
#define MPVIPC_HPP_

#include <cstring>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
using std::string;

const char *DEFAULT_SOCK = "/tmp/mpvsocket";

namespace oim {

/*
 * The class oim::ipc provides easy communication and basic socket management 
 * for any running mpv instance configured to receive commands over a JSON-IPC server/socket.
 */
class ipc {
private:
    sockaddr_un sockaddress;
    int sockfd;
    int socklen;

public:
    ipc() : ipc(DEFAULT_SOCK) {};
    ipc(const char *sockpath);
    ~ipc();

    bool send(string cmd);
};

/*
 * Constructor for oim::ipc
 */
ipc::ipc(const char *sockpath) {
    this->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    this->sockaddress.sun_family = AF_UNIX;
    std::strcpy(this->sockaddress.sun_path, sockpath);
    this->socklen = sizeof(this->sockaddress);

    connect(this->sockfd, (const sockaddr*)&this->sockaddress, this->socklen);
}

/*
 * Destructor for oim::ipc
 */
ipc::~ipc() {
    close(this->sockfd);
}

/*
 * Sends a raw command string to the internal socket at DEFAULT_SOCK
 */
bool ipc::send(string cmd) {
    return write(this->sockfd, cmd.c_str(), cmd.length()) != -1;
}

} // namespace name

#endif