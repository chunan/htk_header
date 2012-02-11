CC = g++
CFLAGS = -Wall -Werror

vpath %.cpp src

.PHONY: all clean

TARGET = bin/HHeader

all: CFLAGS += -O2

debug: CFLAGS += -g

all: $(TARGET)

debug: $(TARGET)


bin/HHeader: HHeader.cpp
	mkdir -p bin
	${CC} ${CFLAGS} $(LDPATH) $< ${LDFLAGS} -o $@

clean:
	rm -f bin/HHeader
