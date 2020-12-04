#ifndef MPVOPTS_HPP_
#define MPVOPTS_HPP_

#include <curl/curl.h>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
using std::string;

string url_decode(const string encoded);
string query_value(string query, string key);

class mpvoptions {
private:
    string url;
    bool fullscreen;
    bool pip;
    bool enqueue;
    bool new_window;

    CURLU *curlu;
public:
    mpvoptions();
    ~mpvoptions();

    string build_cmd();
    string build_ipc();
    void parse(const char *url);

    bool needs_ipc();
};

mpvoptions::mpvoptions() {
    this->curlu = curl_url();
    this->fullscreen = false;
    this->pip = false;
    this->enqueue = false;
}

mpvoptions::~mpvoptions() {
    curl_url_cleanup(this->curlu);
}

/*
 * Builds a command used to invoke mpv with the appropriate arguments
 */
string mpvoptions::build_cmd() {
    std::ostringstream ret;

    ret << "mpv ";
    if (this->fullscreen) ret << "--fs ";
    if (this->pip) ret << "--ontop --no-border --autofit=384x216 --geometry=98\%:98\% ";
    // NOTE: this is not needed for mpv (it always opens a new window), maybe for other players?
    // if (this->new_window) ret << "--new-window";
    ret << this->url;

    return ret.str();
}

string mpvoptions::build_ipc() {
    std::ostringstream ret;

    if (!this->needs_ipc()) return "";

    // TODO: in the future this will need a serious json serializer for more complicated commands
    // Syntax: {"command": ["loadfile", "%s", "append-play"]}\n
    ret << R"({"command": ["loadfile", ")" << this->url << R"(", "append-play"]})" << std::endl;

    return ret.str();
}

/*
 * Parse a URL and populate the current MpvOptions (uses libcurl for parsing)
 */
void mpvoptions::parse(const char *url) {
    curl_url_set(this->curlu, CURLUPART_URL, url, CURLU_NO_DEFAULT_PORT | CURLU_NON_SUPPORT_SCHEME | CURLU_NO_AUTHORITY);

    // Check wether the url contains the right scheme or not
    char *scheme;
    curl_url_get(this->curlu, CURLUPART_SCHEME, &scheme, 0);
    if (std::strcmp(scheme, "mpv")) {
        curl_free(scheme);
        throw string("Unsupported protocol supplied");
    }
    curl_free(scheme);

    // NOTE: libcurl really doesn't like "malformed" url's such as `mpv:///open?xxxxx`
    // and it messes up parsing path and host: HOST becomes `open` and PATH becomes `/`
    // so we just check HOST instead of PATH for the correct method
    char *method;
    curl_url_get(this->curlu, CURLUPART_HOST, &method, 0);
    if (std::strcmp(method, "open")) {
        curl_free(method);
        throw string("Unsupported method supplied");
    }
    curl_free(method);

    // Check wether the url query is empty or not
    char *query;
    curl_url_get(this->curlu, CURLUPART_QUERY, &query, 0);
    if (!query) {
        curl_free(query);
        throw string("Empty query");
    }

    // If the query is not empty, parse it and populate the current object
    string querystr(query);
    curl_free(query);
    this->url = url_decode(query_value(querystr, "url"));
    this->fullscreen = query_value(querystr, "fullscreen") == "1";
    this->pip = query_value(querystr, "pip") == "1";
    this->enqueue = query_value(querystr, "enqueue") == "1";
    this->new_window = query_value(querystr, "new_window") == "1";
}

/*
 * Checks wether or not MpvOptions needs to communicate with mpv via IPC
 * instead of the command line interface
 */
bool mpvoptions::needs_ipc() {
    // For now this is needed only when queuing videos
    return this->enqueue;
}

/*
 * Percent-decodes a URL using curl_easy_unescape
 */
string url_decode(const string encoded) {
    CURL *curl = curl_easy_init();
    std::unique_ptr<char, void(*)(char*)> url_decoded(
        curl_easy_unescape(
            curl,
            encoded.c_str(),
            (int) encoded.length(),
            nullptr
        ),
        [](char *ptr) { curl_free(ptr); }
    );

    return string(url_decoded.get());
}

/*
 * Gets a value from a query string given a key
 */
string query_value(string query, string key) {
    // Find the beginning of the last occurrence of `key` in `query`
    auto pos = query.rfind(key + "=");
    if (pos == string::npos) return "";

    // Offset calculation (beginning of the value string associated with `key`):
    //  pos: positione of the first character of `key`
    //  key.length(): self explanatory
    //  1: length of character '='
    int offset = pos + key.length() + 1;
    // Return a string starting from the offset and with appropriate length
    // (difference between the position of the first '&' char after the value and `offset`)
    return query.substr(offset, query.find('&', pos) - offset);
}

#endif