package open_in_mpv

import (
	"net/url"
	"strings"
	"testing"
)

var fakePlayer = Player{
	Name:             "FakePlayer",
	Executable:       "fakeplayer",
	Fullscreen:       "",
	Pip:              `--ontop --no-border --autofit=384x216 --geometry=98%:98%`,
	Enqueue:          "",
	NewWindow:        "",
	NeedsIpc:         true,
	SupportedSchemes: []string{"https"},
	FlagOverrides:    map[string]string{},
}

func testUrl(query ...string) string {
	elems := []string{
		`mpv:///open?url=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3DdQw4w9WgXcQ`,
	}
	for _, elem := range query {
		key, value, _ := strings.Cut(elem, "=")
		elems = append(elems, url.QueryEscape(key)+"="+url.QueryEscape(value))
	}
	return strings.Join(elems, "&")
}

func Test_GenerateCommand(t *testing.T) {
	defaultConfig.Players["fakeplayer"] = fakePlayer

	o := NewOptions()
	err := o.Parse(testUrl("player=fakeplayer", "flags=--ytdl-format=bestvideo[height<=480]+bestaudio", "pip=1"))
	if err != nil {
		t.Error(err)
	}

	executable, args := o.GenerateCommand()
	t.Logf("%s %v", executable, args)

	if executable != fakePlayer.Executable {
		t.Logf("expected the default player to be %s", fakePlayer.Executable)
		t.Fail()
	}
	// We expect 6 args: 1 from o.Flags, 4 from o.Pip and 1 is o.Url
	if args == nil || len(args) < 6 {
		t.Logf("expected 6 args, got %d", len(args))
		t.Fail()
	}
}

func Test_GenerateIPC(t *testing.T) {
	o := NewOptions()
	_ = o.Parse(testUrl("enqueue=1", "pip=1"))
	ipc, _ := o.GenerateIPC()
	t.Log(string(ipc))
}

func Test_overrideFlags_single(t *testing.T) {
	fakePlayer.FlagOverrides["--foo"] = "--bar=%s"
	defaultConfig.Players["fakeplayer"] = fakePlayer

	o := NewOptions()
	err := o.Parse(testUrl("player=fakeplayer", "flags=--foo"))
	if err != nil {
		t.Error(err)
	}

	result := o.overrideFlags()

	if result != "--bar=foo" {
		t.Fail()
		t.Log(result)
		t.Logf("%#v\n", o)
	}
}

func Test_overrideFlags_star(t *testing.T) {
	fakePlayer.FlagOverrides["*"] = "--bar=%s"
	defaultConfig.Players["fakeplayer"] = fakePlayer

	o := NewOptions()
	err := o.Parse(testUrl("player=fakeplayer", "flags=--foo%20--baz"))
	if err != nil {
		t.Error(err)
	}

	result := o.overrideFlags()

	if result != "--bar=foo --bar=baz" {
		t.Fail()
		t.Log(result)
		t.Logf("%#v\n", o)
	}
}

func Test_Parse(t *testing.T) {
	fakePlayer.FlagOverrides["*"] = "--bar=%s"
	defaultConfig.Players["fakeplayer"] = fakePlayer

	o := NewOptions()
	err := o.Parse(testUrl("player=fakeplayer", "enqueue=1", "pip=1"))
	if err != nil {
		t.Fatal(err)
	}

	fakePlayer.SupportedSchemes = []string{}
	defaultConfig.Players["fakeplayer"] = fakePlayer
	err = o.Parse(testUrl("player=fakeplayer", "enqueue=1", "pip=1"))
	if err == nil {
		t.Logf("%#v", defaultConfig.Players["fakeplayer"])
		t.Fatal("Err should not be nil")
	}

	executable, args := o.GenerateCommand()
	t.Logf("%s %v", executable, args)
}

func Test_sliceContains(t *testing.T) {
	schemas := []string{
		"http",
		"https",
		"ftp",
		"ftps",
	}

	if !stringSliceContains("https", schemas) {
		t.Logf("should return true if element (https) is in slice (%v)", schemas)
		t.Fail()
	}

	if stringSliceContains("av", schemas) {
		t.Logf("should return false if element (av) is not in slice (%v)", schemas)
		t.Fail()
	}
}
