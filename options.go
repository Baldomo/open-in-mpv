package open_in_mpv

import (
	"encoding/json"
	"fmt"
	"net/url"
	"strings"
)

// The Options struct defines a model for the data contained in the mpv://
// URL and acts as a command generator (both CLI and IPC) to spawn and
// communicate with an mpv player window.
type Options struct {
	Flags      string
	Player     string
	Url        string
	Enqueue    bool
	Fullscreen bool
	NewWindow  bool
	Pip        bool
	NeedsIpc   bool
}

// Utility object to marshal an mpv-compatible JSON command. As defined in the
// documentation, a valid IPC command is a JSON array of strings
type enqueueCmd struct {
	Command []string `json:"command,omitempty"`
}

// Default constructor for an Option object
func NewOptions() Options {
	return Options{
		Flags:      "",
		Player:     "mpv",
		Url:        "",
		Enqueue:    false,
		Fullscreen: false,
		NewWindow:  false,
		Pip:        false,
		NeedsIpc:   false,
	}
}

// Parse a mpv:// URL and populate the current Options
func (o *Options) Parse(uri string) error {
	u, err := url.Parse(uri)
	if err != nil {
		return err
	}

	if u.Scheme != "mpv" {
		return fmt.Errorf("Unsupported protocol: %s", u.Scheme)
	}

	if u.Path != "/open" {
		return fmt.Errorf("Unsupported method: %s", u.Path)
	}

	if len(u.RawQuery) < 2 {
		return fmt.Errorf("Empty or malformed query: %s", u.RawQuery)
	}

	o.Flags, err = url.QueryUnescape(u.Query().Get("flags"))
	if err != nil {
		return err
	}
	o.Url, err = url.QueryUnescape(u.Query().Get("url"))
	if err != nil {
		return err
	}

	if p, ok := u.Query()["player"]; ok {
		o.Player = p[0]
	}

	if GetPlayerInfo(o.Player) == nil {
		return fmt.Errorf("Unsupported player: %s", o.Player)
	}

	o.Enqueue = u.Query().Get("enqueue") == "1"
	o.Fullscreen = u.Query().Get("fullscreen") == "1"
	o.NewWindow = u.Query().Get("new_window") == "1"
	o.Pip = u.Query().Get("pip") == "1"

	o.NeedsIpc = GetPlayerInfo(o.Player).NeedsIpc

	return nil
}

// Parses flag overrides and returns the final flags
func (o Options) overrideFlags() string {
	var (
		ret  []string
		star bool
	)

	pInfo := GetPlayerInfo(o.Player)
	if pInfo == nil {
		return ""
	}

	_, star = pInfo.FlagOverrides["*"]

	for _, flag := range strings.Split(o.Flags, " ") {
		if star {
			stripped := strings.TrimLeft(flag, "-")
			replaced := strings.ReplaceAll(pInfo.FlagOverrides["*"], `%s`, stripped)
			ret = append(ret, replaced)
		} else {
			if override, ok := pInfo.FlagOverrides[flag]; ok {
				stripped := strings.TrimLeft(flag, "-")
				ret = append(ret, strings.ReplaceAll(override, `%s`, stripped))
			}
		}
	}

	return strings.Join(ret, " ")
}

// Builds a CLI command used to invoke the player with the appropriate arguments
func (o Options) GenerateCommand() []string {
	var ret []string

	pInfo := GetPlayerInfo(o.Player)

	if o.Fullscreen {
		ret = append(ret, pInfo.Fullscreen)
	}

	if o.Pip {
		ret = append(ret, pInfo.Pip)
	}

	if o.Flags != "" {
		if len(pInfo.FlagOverrides) == 0 {
			ret = append(ret, o.Flags)
		} else {
			ret = append(ret, o.overrideFlags())
		}
	}

	ret = append(ret, o.Url)

	return ret
}

// Builds the IPC command needed to enqueue videos if the player requires it
func (o Options) GenerateIPC() ([]byte, error) {
	if !o.NeedsIpc {
		return []byte{}, fmt.Errorf("This player does not need IPC")
	}

	cmd := enqueueCmd{
		[]string{"loadfile", o.Url, "append-play"},
	}

	ret, err := json.Marshal(cmd)
	if err != nil {
		return []byte{}, err
	}

	if ret[len(ret)-1] != '\n' {
		ret = append(ret, '\n')
	}

	return ret, nil
}
