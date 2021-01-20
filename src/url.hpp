#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

using std::string;

namespace {
/*
 * Converts a single character to a percent-decodable byte representation
 * Taken from
 * https://github.com/cpp-netlib/url/blob/main/include/skyr/v1/percent_encoding/percent_decode_range.hpp
 */
inline std::byte alnum_to_hex(char value) {
    if ((value >= '0') && (value <= '9')) {
        return static_cast<std::byte>(value - '0');
    }

    if ((value >= 'a') && (value <= 'f')) {
        return static_cast<std::byte>(value + '\x0a' - 'a');
    }

    if ((value >= 'A') && (value <= 'F')) {
        return static_cast<std::byte>(value + '\x0a' - 'A');
    }

    return static_cast<std::byte>(' ');
}

} // namespace

namespace oim {

/*
 * The class oim::url contains utility methods to parse a URL string and
 * access its fields by name (e.g. parses protocol, host, query etc.). Simple
 * query value searching by key is also provided by url::query_value(string).
 */
class url {
  private:
    string protocol_, host_, path_, query_;

  public:
    /* Constructor with C-style string URL */
    url(const char *url_s) : url(string(url_s)){};

    /* Constructor with C++ std::string URL */
    url(const string &url_s);

    /* Move constructor for oim::url */
    url(url &&other);

    /* Accessor for the URL's protocol string */
    string protocol() { return protocol_; }

    /* Accessor for the URL's host string */
    string host() { return host_; }

    /* Accessor for the URL's whole path */
    string path() { return path_; }

    /* Accessor for the URL's whole query string */
    string query() { return query_; }

    /*
     * Gets a value from a query string given a key
     */
    string query_value(string key);

    /*
     * Gets a value from a query string given a key (overload with optional
     * fallback if value isn't found)
     */
    string query_value(string key, string fallback);
};

url::url(const string &url_s) {
    const string prot_end("://");
    string::const_iterator prot_i = std::search(
        url_s.begin(), url_s.end(), prot_end.begin(), prot_end.end());
    protocol_.reserve(std::distance(url_s.begin(), prot_i));
    // The protocol is case insensitive
    std::transform(
        url_s.begin(), prot_i, std::back_inserter(protocol_), ::tolower);
    if (prot_i == url_s.end())
        return;
    std::advance(prot_i, prot_end.length());

    // The path starts with '/'
    string::const_iterator path_i = std::find(prot_i, url_s.end(), '/');
    host_.reserve(std::distance(prot_i, path_i));
    // The host is also case insensitive
    std::transform(prot_i, path_i, std::back_inserter(host_), ::tolower);

    // Everything else is query
    string::const_iterator query_i = std::find(path_i, url_s.end(), '?');
    path_.assign(path_i, query_i);
    if (query_i != url_s.end())
        ++query_i;
    query_.assign(query_i, url_s.end());
}

url::url(url &&other) {
    protocol_ = std::move(other.protocol_);
    host_ = std::move(other.host_);
    path_ = std::move(other.path_);
    query_ = std::move(other.query_);
}

string url::query_value(string key) {
    // Find the beginning of the last occurrence of `key` in `query`
    auto pos = query_.rfind(key + "=");
    if (pos == string::npos)
        return "";

    // Offset calculation (beginning of the value string associated with
    // `key`):
    //    pos: positione of the first character of `key`
    //    key.length(): self explanatory
    //    1: length of character '='
    int offset = pos + key.length() + 1;
    // Return a string starting from the offset and with appropriate length
    // (difference between the position of the first '&' char after the
    // value and `offset`)
    return query_.substr(offset, query_.find('&', pos) - offset);
}

string url::query_value(string key, string fallback) {
    string ret = query_value(key);
    if (ret.empty())
        return fallback;
    return ret;
}

/*
 * Percent-decodes a URL
 */
string percent_decode(const string encoded) {
    string ret = "";
    for (auto i = encoded.begin(); i < encoded.end(); i++) {
        if (*i == '%') {
            std::byte b1 = ::alnum_to_hex(*++i);
            std::byte b2 = ::alnum_to_hex(*++i);

            char parsed = static_cast<char>(
                (0x10u * std::to_integer<unsigned int>(b1)) +
                std::to_integer<unsigned int>(b2));
            ret += parsed;
        } else {
            ret += *i;
        }
    }

    return ret;
}

} // namespace oim