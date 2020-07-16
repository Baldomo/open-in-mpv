# open-in-mpv
This is a simple web extension (for Chrome and Firefox) which helps open any video in the currently open tab in the [mpv player](https://mpv.io).

The extension itself is a copy of the one from the awesome [iina](https://github.com/iina/iina), while the (bare) backend is written in C++11 (this is a rewrite from Rust).

## Installation
> Compiled binaries and packed extensions can be found in the [releases page](https://github.com/Baldomo/open-in-mpv/releases).

To install `open-in-mpv`, just run
```sh
sudo make install
```

### The `mpv://` protocol
`open-in-mpv install-protocol` will create a custom `xdg-open` desktop file with a scheme handler for the `mpv://` protocol. This lets `xdg-open` call `open-in-mpv` with an encoded URI, so that it can be parsed and the information can be relayed to `mpv` - this logic follows how `iina` parses and opens custom URIs with the `iina://` protocol on Mac. `install-protocol.sh` has the same functionality.