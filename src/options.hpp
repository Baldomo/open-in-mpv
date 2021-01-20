#pragma once

#include "players.hpp"
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
    player *player_info_;
    string url_;
    string flags_;
    string player_;
    bool fullscreen_;
    bool pip_;
    bool enqueue_;
    bool new_window_;

  public:
    /*
     * Constructor for oim::options
     */
    options();

    /*
     * Builds a CLI command used to invoke mpv with the appropriate arguments
     */
    string build_cmd();

    /*
     * Builds the IPC command needed to enqueue videos in mpv
     */
    string build_ipc();

    /*
     * Parse a URL and populate the current oim::options
     */
    void parse(const char *url);

    /*
     * Checks wether or not oim::options needs to communicate with mpv via IPC
     * instead of the command line interface
     */
    bool needs_ipc();
};

options::options() {
    url_ = "";
    flags_ = "";
    player_ = "mpv";
    fullscreen_ = false;
    pip_ = false;
    enqueue_ = false;
    new_window_ = false;
}

string options::build_cmd() {
    std::ostringstream ret;

    if (player_info_ == nullptr) {
        return "";
    }

    ret << player_info_->executable << " ";
    if (fullscreen_)
        ret << player_info_->fullscreen << " ";
    if (pip_)
        ret << player_info_->pip << " ";
    if (!flags_.empty())
        ret << flags_ << " ";
    ret << url_;

    return ret.str();
}

string options::build_ipc() {
    std::ostringstream ret;

    if (!needs_ipc())
        return "";

    // In the future this may need a more serious json serializer for
    // more complicated commands
    // Syntax: {"command": ["loadfile", "%s", "append-play"]}\n
    ret << R"({"command": ["loadfile", ")" << url_ << R"(", "append-play"]})"
        << std::endl;

    return ret.str();
}

void options::parse(const char *url_s) {
    oim::url u(url_s);

    if (u.protocol() != "mpv")
        throw string("Unsupported protocol supplied: ") + u.protocol();

    if (u.path() != "/open")
        throw string("Unsupported method supplied: ") + u.path();

    if (u.query().empty())
        throw string("Empty query");

    url_ = percent_decode(u.query_value("url"));
    flags_ = percent_decode(u.query_value("flags"));
    player_ = u.query_value("player", "mpv");

    player_info_ = get_player_info(player_);
    if (player_info_ == nullptr)
        throw string("Unsupported player: ") + player_;

    fullscreen_ = u.query_value("fullscreen") == "1";
    pip_ = u.query_value("pip") == "1";
    enqueue_ = u.query_value("enqueue") == "1";
    new_window_ = u.query_value("new_window") == "1";
}

bool options::needs_ipc() {
    // For now this is needed only when queuing videos
    return enqueue_;
}

} // namespace oim