CXXFLAGS=$(shell pkg-config --cflags gtk+-3.0)
CXXFLAGS+= -std=c++11 -rdynamic -O2 -I include
LDFLAGS=$(shell pkg-config --libs gtk+-3.0)

all: $(basename $(wildcard src/*.cpp))

clean:
	$(RM) $(basename $(wildcard src/*.cpp))
