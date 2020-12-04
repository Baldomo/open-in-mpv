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

The table below is a simple documentation of the URL query keys and values used to let the `open-in-mpv` executable what to do:

| Key           | Example value                          | Description |
|---------------|----------------------------------------|---------------|
| `url`         | `https%3A%2F%2Fyoutu.be%2FdQw4w9WgXcQ` | The actual file URL to be played, URL-encoded |
| `full_screen` | `1`                                    | Controls whether the video is played in fullscreen mode |
| `pip`         | `1`                                    | Simulates a picture-in-picture mode (only works with mpv for now) |
| `enqueue`     | `1`                                    | Adds a video to the queue (see below) |
| `new_window`  | `1`                                    | Forcibly starts a video in a new window even if one is already open |
| `player`      | `celluloid`                            | :warning: WIP, starts any arbitrary video player (only mpv-based ones are recommended, such as [Celluloid](https://celluloid-player.github.io/)) |
| `flags`       | `--vo%3Dgpu`                           | Custom command options and flags to be passed to the video player |

### Playlist and `enqueue` functionality
For `enqueue` to work properly with any mpv-based player, the playes has to be able to read commands from a `fifo` name pipe. This can be achieved by adding the following lines to any of the user's startup utilities/files, e.g. `.bashrc`, `.zshrc` or `.profile`.

```sh
mpvsocket=/tmp/mpvsocket
[[ ! -p /tmp/mpvsocket ]] && mkfifo /tmp/mpvsocket
```