#ifndef MPVIPC_HPP_
#define MPVIPC_HPP_

#include <cstring>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
using std::string;

const char *DEFAULT_SOCK = "/tmp/mpvsocket";

class mpvipc {
private:
    sockaddr_un sockaddress;
    int sockfd;
    int socklen;
public:
    mpvipc() : mpvipc(DEFAULT_SOCK) {};
    mpvipc(const char *sockpath);
    ~mpvipc();

    bool send(string cmd);
};

mpvipc::mpvipc(const char *sockpath) {
    this->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    this->sockaddress.sun_family = AF_UNIX;
    std::strcpy(this->sockaddress.sun_path, sockpath);
    this->socklen = sizeof(this->sockaddress);

    connect(this->sockfd, (const sockaddr*)&this->sockaddress, this->socklen);
}

mpvipc::~mpvipc() {
    close(this->sockfd);
}

bool mpvipc::send(string cmd) {
    return write(this->sockfd, cmd.c_str(), cmd.length()) != -1;
}

#endif