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
	Pip:              "",
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
	return strings.Join(append(elems, query...), "&")
}

func Test_GenerateCommand(t *testing.T) {
	o := NewOptions()
	o.Url, _ = url.Parse("example.com")
	o.Flags = "--vo=gpu"
	o.Pip = true

	args := o.GenerateCommand()
	t.Logf("%s %v", o.Player, args)
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

	args := o.GenerateCommand()
	t.Logf("%s %v", o.Player, args)
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
