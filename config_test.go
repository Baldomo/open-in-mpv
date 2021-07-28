package open_in_mpv

import "testing"

func Test_LoadConfig(t *testing.T) {
	err := LoadConfig()
	if err != nil {
		t.Error(err)
	}
	t.Logf("%#v\n", defaultConfig)
}
