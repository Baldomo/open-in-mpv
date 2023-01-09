<h1 align="center">open-in-mpv</h1>

<p align="center">
<a href="https://github.com/Baldomo/open-in-mpv/releases/latest" alt="Chromium extension (signed)"><img src="images/badge-chromium.png"></a>
<a href="https://addons.mozilla.org/addon/iina-open-in-mpv/" alt="Firefox extension (AMO)"><img src="images/badge-firefox.png"></a>
</p>

---

This is a simple web extension (for Chrome and Firefox) which helps open any video in the currently open tab in the [mpv player](https://mpv.io).

The extension itself shares a lot of code with the one from the awesome [iina](https://github.com/iina/iina), while the (bare) native binary is written in Go (this is a rewrite from C++).

- [Installation](#installation)
- [Configuration](#configuration)
- [The `mpv://` protocol](#the-mpv-protocol)
- [Playlist and `enqueue` functionality](#playlist-and-enqueue-functionality)
- [Player support](#player-support)
- [Supported protocols](#supported-protocols)

### Installation
> Compiled binaries and packed extensions can be found in the [releases page](https://github.com/Baldomo/open-in-mpv/releases).

To build and install `open-in-mpv` and the `mpv://` protocol handler, just run

```
$ sudo make install
$ make install-protocol
```

### Configuration
The configuration file has to be named `config.yml` and can be placed in the same folder as the executable or in:

|                            | Windows                            | Linux/BSDs                     | MacOS                                              |   |
| -------------------------- | ---------------------------------- | ------------------------------ | -------------------------------------------------- | - |
| System level configuration | `C:\ProgramData\open-in-mpv\`      | `/etc/xdg/open-in-mpv/`        | `/Library/Application Support/open-in-mpv/`        |   |
| User level configuration   | `C:\Users\<User>\AppData\Roaming\` | `${HOME}/.config/open-in-mpv/` | `${HOME}/Library/Application Support/open-in-mpv/` |   |

The user level configuration will always override the system level one.

The configuration file has the following structure:

```yaml
fake:                  # open_in_mpv.Player
  name:                # string
  executable:          # string
  fullscreen:          # string
  pip:                 # string
  enqueue:             # string
  new_window:          # string
  needs_ipc:           # true | false
  supported_protocols: # []string
  flag_overrides:      # map[string]string
```

> See [the default configuration](config.yml) or the [example](#example) below

And the `open_in_mpv.Player` object is defined as follows:

| Key                   | Example value       | Description                                                                                                                 |
| --------------------- | ------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| `name`                | `mpv`               | Full name of the video player; not used internally                                                                          |
| `executable`          | `mpv`               | The player's binary path/name (doesn't need full path if already in `$PATH`)                                                |
| `fullscreen`          | `"--fs"`            | Flag override to open the player in fullscreen (can be empty)                                                               |
| `pip`                 | `"--pip"`           | Flag override to open the player in picture-in-picture mode (can be empty)                                                  |
| `enqueue`             | `"--enqueue"`       | Flag override to add the video to the player's queue (can be empty)                                                         |
| `new_window`          | `"--new-window"`    | Flag override to force open a new player window with the video (can be empty)                                               |
| `needs_ipc`           | `false`             | Controls whether the player needs IPC communication (only generates mpv-compatible JSON commands, used for enqueing videos) |
| `supported_protocols` | `["http", "https"]` | An arbitrary whitelist of protocols the player supports. See the [relevant section](#supported-protocols)                   |
| `flag_overrides`      | `"*": "--mpv-%s"`   | Defines arbitrary text overrides for command line flags (see below)                                                         |

#### Flag overrides

The configuration for a player can contain overrides for command line flags defined in the web extension's configuration page, so not to make a different player crash/not start because the flags are not accepted. With the correct overrides any kind of flag passed in [the URL](#the-mpv-protocol) by the extension will either be ignored or replaced with a pattern/prefix/suffix so that it becomes valid for the player in use.

- `"*"`: matches anything and will take precedence over any other override 
  - e.g. the pair `"*": ""` will void all flags
- `"flag"`: matches the flag `--flag` 
  - e.g. the pair `"--foo": "--bar"` will replace `--foo` with `--bar`
- `"%s"`: is replaced with the original flag without any leading dash 
  - e.g. the pair `"--foo": "--%s-bar"` will replace `--foo` with `--foo-bar`

> Note: command line options with parameters such as `--foo=bar` are considered as a whole, single flag

Celluloid, for example, expects all mpv-related command line flags to be prefixed with `--mpv-`, so its configuration will contain the following overrides:

```yaml
flag_overrides: 
  "*": "--mpv-%s"
```

#### Example

This is a full example of a fictitious player definition in the configuration file:

```yaml
players:
  fake:
    name: fake-player
    executable: fakeplayer
    fullscreen: "--fs"
    pip: "--ontop --no-border --autofit=384x216 --geometry=98\\%:98\\%"
    enqueue: "--enqueue"
    new_window: ""
    needs_ipc: true
    supported_protocols:
      - http
      - https
      - ftp
      - ftps
    flag_overrides:
      "*": "--mpv-options=%s"
```

### The `mpv://` protocol
`open-in-mpv install-protocol` will create a custom `xdg-open` desktop file with a scheme handler for the `mpv://` protocol. This lets `xdg-open` call `open-in-mpv` with an encoded URI, so that it can be parsed and the information can be relayed to `mpv` - this logic follows how `iina` parses and opens custom URIs with the `iina://` protocol on Mac. `install-protocol.sh` has the same functionality.

Please note that this specification is enforced quite strictly, as the program will error out when at least one of the following conditions is true:

- The protocol is not `mpv://`
- The method/path is not `/open`
- The query is empty

The table below is a simple documentation of the URL query keys and values used to let the `open-in-mpv` executable what to do.

| Key           | Example value                          | Description                                                                    |
| ------------- | -------------------------------------- | ------------------------------------------------------------------------------ |
| `url`         | `https%3A%2F%2Fyoutu.be%2FdQw4w9WgXcQ` | The actual file URL to be played, URL-encoded                                  |
| `full_screen` | `1`                                    | Controls whether the video is played in fullscreen mode                        |
| `pip`         | `1`                                    | Simulates a picture-in-picture mode (only works with mpv for now)              |
| `enqueue`     | `1`                                    | Adds a video to the queue (see below)                                          |
| `new_window`  | `1`                                    | Forcibly starts a video in a new window even if one is already open            |
| `player`      | `celluloid`                            | Starts any supported video player (see [Player support](#player-support))      |
| `flags`       | `--vo%3Dgpu`                           | Custom command options and flags to be passed to the video player, URL-encoded |

### Playlist and `enqueue` functionality
For `enqueue` to work properly with any mpv-based player (provided it supports mpv's IPC), the player has to read commands from a socket. This can be achieved by adding the following line to the video player's configuration (usually `$HOME/.config/mpv/mpv.conf` for mpv).

```conf
input-ipc-server=/tmp/mpvsocket
```

### Player support
Supported players are defined in `config.yml`, where the struct `Player` ([see `config.go`](config.go)) defines supported functionality and command line flag overrides. To request support for a player you're welcome to open a new issue or a pull request or just add your own in your configuration file.

### Supported protocols
Since opening an arbitrary URL with a shell command can cause remote code execution on the host machine (for example by loading arbitrary `.so` files on a player by using special [schemes](https://en.wikipedia.org/wiki/List_of_URI_schemes)), only protocols/[schemes](https://en.wikipedia.org/wiki/List_of_URI_schemes) explicitly specified in the configuration will be processed by the native binary without errors. Defaults to `["http", "https"]` if empty. There is also no special instructions parsing or catch-all values.