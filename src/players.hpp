#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

namespace oim {

/*
 * Struct `oim::player` contains useful informations for an mpv-based player,
 * such as binary name and fullscreen/pip/enqueue/new_window flags overrides for
 * use in `options::build_ipc()`. A way to override any generic flags is also
 * provided through the map `flag_overrides`.
 */
struct player {
    string name;
    string executable;

    string fullscreen;
    string pip;
    string enqueue;
    string new_window;

    bool needs_ipc;

    /*
     * Override syntax:
     * `"*"`:    matches anything and will take precedence over any other
     * override e.g. the pair `{"*", ""}` will void all flags
     * `"flag"`: matches the flag `--flag`
     *           e.g. the pair `{"foo", "bar"}` will replace `--foo` with
     * `--bar`
     * `"%s"`:   is replaced with the original flag without the leading `--`
     *           e.g. the pair `{"foo", "--%s-bar"}` will replace `--foo` with
     * `--foo-bar`
     *
     * Note: command line options with parameters such as --foo=bar are
     * considered a flags as a whole
     */
    unordered_map<string, string> flag_overrides;
};

unordered_map<string, player> player_info = {
    { "mpv",
      { .name = "mpv",
        .executable = "mpv",
        .fullscreen = "--fs",
        .pip = "--ontop --no-border --autofit=384x216 --geometry=98\%:98\%",
        .enqueue = "",
        .new_window = "",
        .needs_ipc = true,
        .flag_overrides = {} } },
    { "celluloid",
      { .name = "Celluloid",
        .executable = "celluloid",
        .fullscreen = "",
        .pip = "",
        .enqueue = "--enqueue",
        .new_window = "--new-window",
        .needs_ipc = false,
        .flag_overrides = { { "*", "--mpv-options=%s" } } } }
};

player *get_player_info(string name) {
    if (name.empty())
        return &player_info["mpv"];

    string lower_name(name);
    std::transform(name.begin(), name.end(), lower_name.begin(), ::tolower);

    auto info = player_info.find(lower_name);
    if (info == player_info.end()) {
        return nullptr;
    }

    return &(*info).second;
}

} // namespace oim
