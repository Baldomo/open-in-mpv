INCLUDES = -Isrc/ -I/opt/curl/include
LDFLAGS = -L/opt/curl/lib
LDLIBS = -lcurl
CXXFLAGS = -Wall $(INCLUDES) $(LDFLAGS) $(LDLIBS)
SRCS = src/curl.hpp \
	   src/mpvopts.hpp \
	   src/main.cpp

all:
	$(CXX) $(CXXFLAGS) -o open-in-mpv src/main.cpp

install: all
	cp open-in-mpv /usr/bin

uninstall:
	rm /usr/bin/open-in-mpv

clean:
	rm open-in-mpv