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
	Enqueue    bool
	Flags      string
	Fullscreen bool
	NeedsIpc   bool
	NewWindow  bool
	Pip        bool
	Player     string
	Url        *url.URL
}

// Utility object to marshal an mpv-compatible JSON command. As defined in the
// documentation, a valid IPC command is a JSON array of strings
type enqueueCmd struct {
	Command []string `json:"command,omitempty"`
}

// Default constructor for an Option object
func NewOptions() Options {
	return Options{
		Enqueue:    false,
		Flags:      "",
		Fullscreen: false,
		NeedsIpc:   false,
		NewWindow:  false,
		Pip:        false,
		Player:     "mpv",
		Url:        nil,
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

	playerConfig := GetPlayerConfig(o.Player)
	if playerConfig == nil {
		return fmt.Errorf("Unsupported player: %s", o.Player)
	}

	// Extract player command line flags
	o.Flags, err = url.QueryUnescape(u.Query().Get("flags"))
	if err != nil {
		return err
	}

	// Extract video file URL
	rawUrl, err := url.QueryUnescape(u.Query().Get("url"))
	if err != nil {
		return err
	}
	// Parse the unprocessed URL
	o.Url, err = url.Parse(rawUrl)
	if err != nil {
		return err
	}
	// Validate the raw URL scheme against the configured ones
	if !stringSliceContains(o.Url.Scheme, playerConfig.SupportedSchemes) {
		return fmt.Errorf(
			"Unsupported schema for player '%s': %s. Did you forget to add it in the configuration?",
			playerConfig.Name,
			o.Url.Scheme,
		)
	}

	if p, ok := u.Query()["player"]; ok {
		o.Player = p[0]
	}

	o.Enqueue = u.Query().Get("enqueue") == "1"
	o.Fullscreen = u.Query().Get("fullscreen") == "1"
	o.NewWindow = u.Query().Get("new_window") == "1"
	o.Pip = u.Query().Get("pip") == "1"

	o.NeedsIpc = playerConfig.NeedsIpc

	return nil
}

// Parses flag overrides and returns the final flags
func (o Options) overrideFlags() string {
	var (
		ret  []string
		star bool
	)

	playerConfig := GetPlayerConfig(o.Player)
	if playerConfig == nil {
		return ""
	}

	// Premature look for star override in configuration
	_, star = playerConfig.FlagOverrides["*"]

	for _, flag := range strings.Split(o.Flags, " ") {
		if star {
			// Unconditionally replace all flags with the star template
			stripped := strings.TrimLeft(flag, "-")
			replaced := strings.ReplaceAll(
				playerConfig.FlagOverrides["*"],
				`%s`,
				stripped,
			)
			ret = append(ret, replaced)
			continue
		}

		// Otherwise, iterate over all templates for the current flag and
		// do the necessary string replacements
		if override, ok := playerConfig.FlagOverrides[flag]; ok {
			stripped := strings.TrimLeft(flag, "-")
			ret = append(ret, strings.ReplaceAll(
				override,
				`%s`,
				stripped,
			))
		}
	}

	return strings.Join(ret, " ")
}

// Builds a CLI command used to invoke the player with the appropriate
// arguments
func (o Options) GenerateCommand() (string, []string) {
	var ret []string

	playerConfig := GetPlayerConfig(o.Player)

	if o.Fullscreen {
		ret = append(ret, playerConfig.Fullscreen)
	}

	if o.Pip {
		ret = append(ret, playerConfig.Pip)
	}

	if o.Flags != "" {
		if len(playerConfig.FlagOverrides) == 0 {
			ret = append(ret, o.Flags)
		} else {
			ret = append(ret, o.overrideFlags())
		}
	}

	ret = append(ret, o.Url.String())

	return playerConfig.Executable, ret
}

// Builds the IPC command needed to enqueue videos if the player requires it
func (o Options) GenerateIPC() ([]byte, error) {
	if !o.NeedsIpc {
		return []byte{}, fmt.Errorf("This player does not need IPC")
	}

	cmd := enqueueCmd{
		[]string{"loadfile", o.Url.String(), "append-play"},
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

// Simple linear search for value in slice of strings
func stringSliceContains(value string, v []string) bool {
	for _, elem := range v {
		if elem == value {
			return true
		}
	}

	return false
}
