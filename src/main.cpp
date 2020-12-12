#include "mpvopts.hpp"
#include "mpvipc.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using std::string;

const char *help[2] = {
    "This program is not supposed to be called from the command line!",
    "Call with 'install-protocol' to instal the xdg-compatible protocol file in ~/.local/share/applications/"
};

bool install_protocol() {
    const char *protocol_file = R"([Desktop Entry]
Name=open-in-mpv
Exec=open-in-mpv %u
Type=Application
Terminal=false
NoDisplay=true
MimeType=x-scheme-handler/mpv
)";

    const char *homedir = std::getenv("HOME");
    if (!homedir) return false;

    std::ofstream protfile(string(homedir) + "/.local/share/applications/open-in-mpv.desktop");
    protfile << protocol_file;
    protfile.flush();
    protfile.close();

    return true;
}

int main(int argc, char const *argv[]) {
    if (argc == 1) {
        std::cout << help[0] << std::endl << help[1] << std::endl;
        return 0;
    };

    if (string(argv[1]) == "install-protocol") {
        return install_protocol();
    }

    mpvoptions *mo = new mpvoptions();
    try {
        mo->parse(argv[1]);
    } catch (string err) {
        std::cout << err << std::endl;
        return 1;
    }
    
    if (mo->needs_ipc()) {
        mpvipc *mipc = new mpvipc();
        bool success = mipc->send(mo->build_ipc());
        if (success) {
            return 0;
        }

        std::cout << "Error writing to socket, opening new instance" << std::endl;
    }
    std::system(mo->build_cmd().c_str());

    return 0;
}
