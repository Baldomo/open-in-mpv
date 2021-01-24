#pragma once

#include "players.hpp"
#include "url.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

using std::string;

namespace {
// Replace all occurrences of `from` with `to` in `str`
string replace_all(string str, const string &from, const string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        // Handles case where 'to' is a substring of 'from'
        start_pos += to.length();
    }
    return str;
}

// Remove all characters equals to `c` at the beginning of the string
string lstrip(string str, char c) {
    while (str.at(0) == c)
        str.replace(0, 1, "");
    return str;
}
} // namespace

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

    /*
     * Parses flag overrides and returns the final flags
     */
    string override_flags();

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
    if (fullscreen_ && !player_info_->fullscreen.empty())
        ret << player_info_->fullscreen << " ";
    if (pip_ && !player_info_->pip.empty())
        ret << player_info_->pip << " ";
    if (!flags_.empty()) {
        if (!player_info_->flag_overrides.empty())
            ret << override_flags() << " ";
        else
            ret << flags_ << " ";
    }
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

string options::override_flags() {
    // Return immediatly in case there are no overrides
    if (player_info_->flag_overrides.empty())
        return flags_;

    bool star = false;
    std::ostringstream ret;

    // Check whether there's a global override
    auto star_pair = player_info_->flag_overrides.find("*");
    if (star_pair != player_info_->flag_overrides.end())
        star = true;

    // Turn flags_ into a stream to tokenize somewhat idiomatically
    auto flagstream = std::istringstream{ flags_ };
    string tmp;

    while (flagstream >> tmp) {
        if (star) {
            // Remove all dashes at the beginning of the flag
            string stripped = ::lstrip(tmp, '-');
            ret << ::replace_all((*star_pair).second, "%s", stripped);
        } else {
            // Search for the flag currently being processed
            auto fo = player_info_->flag_overrides.find(tmp);
            if (fo == player_info_->flag_overrides.end())
                continue;

            ret << ::replace_all((*fo).second, "%s", tmp);
        }
    }

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

    fullscreen_ = u.query_value("full_screen") == "1";
    pip_ = u.query_value("pip") == "1";
    enqueue_ = u.query_value("enqueue") == "1";
    new_window_ = u.query_value("new_window") == "1";
}

bool options::needs_ipc() { return player_info_->needs_ipc; }

} // namespace oim