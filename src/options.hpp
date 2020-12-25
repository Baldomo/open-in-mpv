#ifndef MPVOPTS_HPP_
#define MPVOPTS_HPP_

#include "url.hpp"

#include <cstring>
#include <sstream>
#include <string>

using std::string;

namespace oim {

/*
 * The class oim::options defines a model for the data contained in the mpv:// 
 * URL and acts as a command generator (both CLI and IPC) to spawn and 
 * communicate with an mpv player window.
 */
class options {
private:
    string url;
    string flags;
    string player;
    bool fullscreen;
    bool pip;
    bool enqueue;
    bool new_window;

public:
    options();

    string build_cmd();
    string build_ipc();
    void parse(const char *url);

    bool needs_ipc();
};

/*
 * Constructor for oim::options 
 */
options::options() {
    this->url = "";
    this->flags = "";
    this->player = "mpv";
    this->fullscreen = false;
    this->pip = false;
    this->enqueue = false;
}

/*
 * Builds a CLI command used to invoke mpv with the appropriate arguments
 */
string options::build_cmd() {
    std::ostringstream ret;

    // TODO: some of these options work only in mpv and not other players
    // This can be solved by adding a list of some sorts (json/toml/whatever)
    // containing the flags to use for each functionality and each player
    ret << this->player << " ";
    if (this->fullscreen) ret << "--fs ";
    if (this->pip) ret << "--ontop --no-border --autofit=384x216 --geometry=98\%:98\% ";
    if (!this->flags.empty())
        ret << this->flags << " ";
    // NOTE: this is not needed for mpv (it always opens a new window), maybe for other players?
    // if (this->new_window) ret << "--new-window";
    ret << this->url;

    return ret.str();
}

/*
 * Builds the IPC command needed to enqueue videos in mpv
 */
string options::build_ipc() {
    std::ostringstream ret;

    if (!this->needs_ipc()) return "";

    // TODO: in the future this may need a more serious json serializer for 
    // more complicated commands
    // Syntax: {"command": ["loadfile", "%s", "append-play"]}\n
    ret << R"({"command": ["loadfile", ")" 
        << this->url 
        << R"(", "append-play"]})" << std::endl;

    return ret.str();
}

/*
 * Parse a URL and populate the current MpvOptions (uses libcurl for parsing)
 */
void options::parse(const char *url_s) {
    oim::url u(url_s);

    if (u.protocol() != "mpv") 
        throw string("Unsupported protocol supplied: ") + u.protocol();

    if (u.path() != "/open")
        throw string("Unsupported method supplied: ") + u.path();

    if (u.query().empty())
        throw string("Empty query");

    this->url = oim::url_decode(u.query_value("url"));
    this->flags = oim::url_decode(u.query_value("flags"));
    this->player = u.query_value("player", "mpv");
    this->fullscreen = u.query_value("fullscreen") == "1";
    this->pip = u.query_value("pip") == "1";
    this->enqueue = u.query_value("enqueue") == "1";
    this->new_window = u.query_value("new_window") == "1";
}  

/*
 * Checks wether or not oim::options needs to communicate with mpv via IPC
 * instead of the command line interface
 */
bool options::needs_ipc() {
    // For now this is needed only when queuing videos
    return this->enqueue;
}

} // namespace oim

#endif