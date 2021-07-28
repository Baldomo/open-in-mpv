SRC:=config.go ipc.go options.go
EXT_SRC:=$(wildcard extension/Chrome/*) extension/Firefox/manifest.json

all: build/open-in-mpv

build/open-in-mpv: $(SRC)
	@mkdir -p build
	go build -ldflags="-s -w" -o build/open-in-mpv ./cmd/open-in-mpv

build/Firefox.zip: $(EXT_SRC)
	@mkdir -p build
	cp -t extension/Firefox extension/Chrome/{*.html,*.js,*.png,*.css}
	zip -r build/Firefox.zip extension/Firefox/
	@rm extension/Firefox/{*.html,*.js,*.png,*.css}

install: build/open-in-mpv
	cp build/open-in-mpv /usr/bin

install-protocol:
	scripts/install-protocol.sh

uninstall:
	rm /usr/bin/open-in-mpv

clean:
	rm -rf build/*

test:
	go test ./...

.PHONY: all install install-protocol uninstall clean test