SRC:=config.go ipc.go options.go $(wildcard cmd/open-in-mpv/*)
EXT_SRC:=$(wildcard extension/Chrome/*) extension/Firefox/manifest.json
SCRIPTS_DIR:=scripts

all: build/linux.tar build/mac.tar build/windows.tar build/firefox.zip

builddir:
	@mkdir -p build/linux build/windows build/mac

build/linux/open-in-mpv: $(SRC) builddir
	@echo -e "\n# Building for Linux"
	env CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -ldflags="-s -w" -o $@ ./cmd/open-in-mpv
	cp $(SCRIPTS_DIR)/install-protocol.sh $(dir $@)
	cp $(SCRIPTS_DIR)/open-in-mpv.desktop $(dir $@)

build/linux.tar: build/linux/open-in-mpv
	tar cf $@ -C $(dir $@)linux $(notdir $(wildcard build/linux/*))

build/mac/open-in-mpv.app: $(SRC) scripts/Info.plist builddir
	@# See https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html
	@# and https://apple.stackexchange.com/questions/253184/associating-protocol-handler-in-mac-os-x
	@echo -e "\n# Building MacOS app bundle"
	@mkdir -p $@/Contents
	go install github.com/randall77/makefat@latest
	env CGO_ENABLED=0 GOOS=darwin GOARCH=amd64 go build -ldflags="-s -w" -o $(dir $@)/open-in-mpv.amd64 ./cmd/open-in-mpv
	env CGO_ENABLED=0 GOOS=darwin GOARCH=arm64 go build -ldflags="-s -w" -o $(dir $@)/open-in-mpv.arm64 ./cmd/open-in-mpv
	$(GOPATH)/bin/makefat $@/Contents/MacOS/open-in-mpv $(dir $@)/open-in-mpv.amd64 $(dir $@)/open-in-mpv.arm64
	cp config.yml $@/Contents/MacOS/
	cp $(SCRIPTS_DIR)/Info.plist $@/Contents

build/mac.tar: build/mac/open-in-mpv.app
	tar cf $@ -C $(dir $@)/mac open-in-mpv.app

build/windows/open-in-mpv.exe: $(SRC) builddir
	@echo -e "\n# Building for Windows"
	env CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build -ldflags="-s -w -H windowsgui" -o $@ ./cmd/open-in-mpv
	cp $(SCRIPTS_DIR)/install-protocol.reg $(dir $@)

build/windows.tar: build/windows/open-in-mpv.exe
	tar cf $@ -C $(dir $@)windows $(notdir $(wildcard build/windows/*))

build/firefox.zip: $(EXT_SRC)
	@mkdir -p build
	cp extension/Chrome/{*.html,*.js,*.png,*.css} extension/Firefox
	zip -j $@ extension/Firefox/*
	@rm extension/Firefox/{*.html,*.js,*.png,*.css}

install: build/linux/open-in-mpv
	cp build/linux/open-in-mpv /usr/bin

install-protocol:
	$(SCRIPTS_DIR)/install-protocol.sh

uninstall:
	rm /usr/bin/open-in-mpv

clean:
	rm -rf build/*

test:
	go test ./...

.PHONY: all builddir install install-protocol uninstall clean test