#ifndef OIM_IPC_HPP_
#define OIM_IPC_HPP_

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using std::string;

const char *DEFAULT_SOCK = "/tmp/mpvsocket";

namespace oim {

/*
 * The class oim::ipc provides easy communication and basic socket management
 * for any running mpv instance configured to receive commands over a JSON-IPC
 * server/socket.
 */
class ipc {
  private:
    sockaddr_un sockaddress_;
    int sockfd_;
    int socklen_;

  public:
    ipc() : ipc(DEFAULT_SOCK){};
    ipc(const char *sockpath);
    ~ipc();

    bool send(string cmd);
};

/*
 * Constructor for oim::ipc
 */
ipc::ipc(const char *sockpath) {
    sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddress_.sun_family = AF_UNIX;
    std::strcpy(sockaddress_.sun_path, sockpath);
    socklen_ = sizeof(sockaddress_);

    connect(sockfd_, (const sockaddr *)&sockaddress_, socklen_);
}

/*
 * Destructor for oim::ipc
 */
ipc::~ipc() { close(sockfd_); }

/*
 * Sends a raw command string to the internal socket at DEFAULT_SOCK
 */
bool ipc::send(string cmd) {
    return write(sockfd_, cmd.c_str(), cmd.length()) != -1;
}

} // namespace oim

#endif