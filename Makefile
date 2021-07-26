INCLUDES = -Isrc/
CXXFLAGS_debug = -Wall -DDEBUG -g -rdynamic -std=c++2a $(INCLUDES)
CXXFLAGS_release = -Wall -fvisibility=hidden -fvisibility-inlines-hidden -std=c++2a -march=x86-64 -mtune=generic -O3 -pipe -fno-plt $(INCLUDES)
SRCS = src/curl.hpp \
	   src/mpvopts.hpp \
	   src/main.cpp

all: release firefox

release: $(SRCS)
	$(CXX) $(CXXFLAGS_release) -o open-in-mpv src/main.cpp

debug: $(SRCS)
	$(CXX) $(CXXFLAGS_debug) -o open-in-mpv src/main.cpp

install: release
	cp open-in-mpv /usr/bin

uninstall:
	rm /usr/bin/open-in-mpv

firefox:
	cp -t Firefox Chrome/{*.html,*.js,*.png,*.css}
	pushd Firefox && zip ../Firefox.zip * && popd
	@rm Firefox/{*.html,*.js,*.png,*.css}

clean:
	@rm -f open-in-mpv Firefox.zip Chrome.crx

fmt:
	clang-format -i src/*.{hpp,cpp}

.PHONY: all release debug install uninstall firefox clean fmt
