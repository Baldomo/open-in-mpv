package open_in_mpv

import (
	"log"
	"path/filepath"
	"strings"

	"github.com/shibukawa/configdir"
	"gopkg.in/yaml.v2"
)

// The Player struct contains useful informations for an mpv-based player,
// such as binary name and fullscreen/pip/enqueue/new_window flags overrides for
// use in GenerateIPC(). A way to override any generic flags is also
// provided through the map FlagOverrides.
type Player struct {
	// Full name of the player
	Name string
	// Executable path/name (if already in $PATH) for the player
	Executable string
	// Flag override for fullscreen
	Fullscreen string
	// Flag override for picture-in-picture mode
	Pip string
	// Flag override for enqueuing videos
	Enqueue string
	// Flag override for forcing new window
	NewWindow string `yaml:"new_window"`
	// Controls whether this player needs IPC command to enqueue videos
	NeedsIpc bool `yaml:"needs_ipc"`
	// Controls which (video URL) schemes are to be opened by the current
	// player. There is no match-all, each protocol has to be manuall specified
	SupportedSchemes []string `yaml:"supported_protocols"`
	// Overrides for any generic flag
	FlagOverrides map[string]string `yaml:"flag_overrides"`
}

// Top-level configuration object, maps a player by name to its Player object
type Config struct {
	Players map[string]Player
}

var defaultSupportedSchemas = []string{
	"http",
	"https",
}

var defaultConfig = Config{
	Players: map[string]Player{
		"mpv": {
			Name:          "mpv",
			Executable:    "mpv",
			Fullscreen:    "--fs",
			Pip:           `--ontop --no-border --autofit=384x216 --geometry=98%:98%`,
			Enqueue:       "",
			NewWindow:     "",
			NeedsIpc:      true,
			FlagOverrides: map[string]string{},
		},
	},
}

// Tries to load configuration file with fallback to a default configuration
// object
func LoadConfig() error {
	confDirs := configdir.New("", "open-in-mpv")
	confDirs.LocalPath, _ = filepath.Abs(".")
	confFolder := confDirs.QueryFolderContainsFile("config.yml")

	if confFolder == nil {
		log.Println("No config file found, using default")
		return nil
	}

	data, err := confFolder.ReadFile("config.yml")
	if err != nil {
		return err
	}

	err = yaml.Unmarshal(data, &defaultConfig)
	if err != nil {
		return err
	}

	// If the player has no external configuration, use strict defaults
	for name, player := range defaultConfig.Players {
		if len(player.SupportedSchemes) == 0 {
			log.Printf("Player '%s' has no schemas, setting to defaults", player.Name)
			player.SupportedSchemes = defaultSupportedSchemas
			defaultConfig.Players[name] = player
		}
	}

	return nil
}

// Returns player information for the given name if present, otherwise nil
func GetPlayerConfig(name string) *Player {
	lowerName := strings.ToLower(name)
	if p, ok := defaultConfig.Players[lowerName]; ok {
		return &p
	}

	return nil
}
