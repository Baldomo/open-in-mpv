SRC := config.go ipc.go options.go $(wildcard cmd/open-in-mpv/*)
EXT_SRC := $(wildcard extension/Chrome/*) extension/Firefox/manifest.json
SCRIPTS_DIR := scripts
PREFIX := /usr/bin

TAG_COMMIT := $(shell git rev-list --abbrev-commit --tags --max-count=1)
TAG := $(shell git describe --abbrev=0 --tags ${TAG_COMMIT} 2>/dev/null || true)
COMMIT := $(shell git rev-parse --short HEAD)
DATE_PRETTY := $(shell env TZ=UTC0 git log -1 --format=%cd --date=format:"%Y-%m-%d")
DATE := $(shell env TZ=UTC0 git log -1 --format=%cd --date=format:"%Y%m%d")
VERSION := $(TAG:v%=%)
ifneq ($(COMMIT), $(TAG_COMMIT))
    VERSION := $(VERSION)-next-$(COMMIT)-$(DATE)
endif
ifeq ($(VERSION),)
    VERSION := $(COMMIT)-$(DATA)
endif
ifneq ($(shell git status --porcelain),)
    VERSION := $(VERSION)-dirty
endif
BUILD_DIR := build/$(VERSION)

LDFLAGS := -s -w -X main.Version=$(TAG) -X main.BuildDate=$(DATE_PRETTY) -X main.Commit=$(COMMIT)


all: $(BUILD_DIR)/open-in-mpv_$(VERSION).tar.gz $(BUILD_DIR)/linux.tar $(BUILD_DIR)/mac.tar $(BUILD_DIR)/windows.tar $(BUILD_DIR)/firefox.zip

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/linux $(BUILD_DIR)/windows $(BUILD_DIR)/mac

$(BUILD_DIR)/open-in-mpv_$(VERSION).tar.gz: $(SRC)
	@echo -e "\n# Creating tarball"
	go mod vendor
	tar czf $(BUILD_DIR)/open-in-mpv_$(VERSION).tar.gz --transform "s,^,open-in-mpv_$(VERSION)/," --exclude $(BUILD_DIR) *
	rm -rf vendor

$(BUILD_DIR)/linux/open-in-mpv: $(SRC) $(BUILD_DIR)
	@echo -e "\n# Building for Linux"
	env CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -ldflags="$(LDFLAGS)" -o $@ ./cmd/open-in-mpv
	cp $(SCRIPTS_DIR)/install-protocol.sh $(dir $@)
	cp $(SCRIPTS_DIR)/open-in-mpv.desktop $(dir $@)

$(BUILD_DIR)/linux.tar: $(BUILD_DIR)/linux/open-in-mpv
	tar cf $@ -C $(dir $@)linux $(notdir $(wildcard $(BUILD_DIR)/linux/*))

$(BUILD_DIR)/mac/open-in-mpv.app: $(SRC) scripts/Info.plist $(BUILD_DIR)
	@# See https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html
	@# and https://apple.stackexchange.com/questions/253184/associating-protocol-handler-in-mac-os-x
	@echo -e "\n# Building MacOS app bundle"
	@mkdir -p $@/Contents/MacOS
	go install github.com/randall77/makefat@latest
	env CGO_ENABLED=0 GOOS=darwin GOARCH=amd64 go build -ldflags="$(LDFLAGS)" -o $(dir $@)open-in-mpv.amd64 ./cmd/open-in-mpv
	env CGO_ENABLED=0 GOOS=darwin GOARCH=arm64 go build -ldflags="$(LDFLAGS)" -o $(dir $@)open-in-mpv.arm64 ./cmd/open-in-mpv
	$(GOBIN)/makefat $@/Contents/MacOS/open-in-mpv $(dir $@)open-in-mpv.amd64 $(dir $@)open-in-mpv.arm64
	cp config.yml $@/Contents/MacOS/
	cp $(SCRIPTS_DIR)/Info.plist $@/Contents

$(BUILD_DIR)/mac.tar: $(BUILD_DIR)/mac/open-in-mpv.app
	tar cf $@ -C $(dir $@)mac open-in-mpv.app

$(BUILD_DIR)/windows/open-in-mpv.exe: $(SRC) $(BUILD_DIR)
	@echo -e "\n# Building for Windows"
	env CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build -ldflags="$(LDFLAGS) -H windowsgui" -o $@ ./cmd/open-in-mpv
	cp $(SCRIPTS_DIR)/install-protocol.reg $(dir $@)

$(BUILD_DIR)/windows.tar: $(BUILD_DIR)/windows/open-in-mpv.exe
	tar cf $@ -C $(dir $@)windows $(notdir $(wildcard $(BUILD_DIR)/windows/*))

$(BUILD_DIR)/firefox.zip: $(BUILD_DIR) $(EXT_SRC)
	sed -i 's/"version": .*"/"version": '\"$(VERSION)\"'/g' extension/Chrome/manifest.json
	cp extension/Chrome/{*.html,*.js,*.png,*.css} extension/Firefox
	sed -i 's/"version": .*"/"version": '\"$(VERSION)\"'/g' extension/Firefox/manifest.json
	zip -j $@ extension/Firefox/*
	@rm extension/Firefox/{*.html,*.js,*.png,*.css}

install: $(BUILD_DIR)/linux/open-in-mpv
	cp $(BUILD_DIR)/linux/open-in-mpv $(PREFIX)

install-protocol:
	$(SCRIPTS_DIR)/install-protocol.sh

uninstall:
	rm /usr/bin/open-in-mpv

clean:
	rm -rf $(dir $(BUILD_DIR))*

test:
	go test ./...

.PHONY: all install install-protocol uninstall clean test