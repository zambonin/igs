CXXFLAGS=$(shell pkg-config --cflags gtk+-3.0)
CXXFLAGS+= -std=c++11 -rdynamic -O2 -Wall -Wextra -Wno-unused-parameter
LDFLAGS=$(shell pkg-config --libs gtk+-3.0)

all: $(basename $(wildcard *.cpp))

clean:
	@rm $(basename $(wildcard *.cpp))
