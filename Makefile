SRC:=config.go ipc.go options.go $(wildcard cmd/open-in-mpv/*)
EXT_SRC:=$(wildcard extension/Chrome/*) extension/Firefox/manifest.json

all: build/linux.tar build/mac.tar build/windows.tar

builddir:
	@mkdir -p build/linux build/windows build/mac

build/linux/open-in-mpv: $(SRC) builddir
	@echo -e "\n# Building for Linux"
	env GOOS=linux GOARCh=amd64 go build -ldflags="-s -w" -o $@ ./cmd/open-in-mpv
	cp scripts/install-protocol.sh $(dir $@)
	cp scripts/open-in-mpv.desktop $(dir $@)

build/linux.tar: build/linux/open-in-mpv
	tar cf $@ -C $(dir $@)linux $(notdir $(wildcard build/linux/*))

build/mac/open-in-mpv.app: $(SRC) scripts/Info.plist builddir
	@# See https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html
    @# and https://apple.stackexchange.com/questions/253184/associating-protocol-handler-in-mac-os-x
	@echo -e "\n# Building MacOS app bundle"
	@mkdir -p $@/Contents
	env GOOS=darwin GOARCh=amd64 go build -ldflags="-s -w" -o $@/Contents/MacOS/open-in-mpv ./cmd/open-in-mpv
	cp config.yml $@/Contents/MacOS/
	cp scripts/Info.plist $@/Contents

build/mac.tar: build/mac/open-in-mpv.app
	tar cf $@ -C $(dir $@)/mac open-in-mpv.app

build/windows/open-in-mpv.exe: $(SRC) builddir
	@echo -e "\n# Building for Windows"
	env GOOS=windows GOARCh=amd64 go build -ldflags="-s -w -H windowsgui" -o $@ ./cmd/open-in-mpv
	cp scripts/install-protocol.reg $(dir $@)

build/windows.tar: build/windows/open-in-mpv.exe
	tar cf $@ -C $(dir $@)windows $(notdir $(wildcard build/windows/*))

build/firefox.zip: $(EXT_SRC)
	@mkdir -p build
	cp -t extension/Firefox extension/Chrome/{*.html,*.js,*.png,*.css}
	zip -r build/Firefox.zip extension/Firefox/
	@rm extension/Firefox/{*.html,*.js,*.png,*.css}

install: build/linux/open-in-mpv
	cp build/linux/open-in-mpv /usr/bin

install-protocol:
	scripts/install-protocol.sh

uninstall:
	rm /usr/bin/open-in-mpv

clean:
	rm -rf build/*

test:
	go test ./...

.PHONY: all builddir install install-protocol uninstall clean test