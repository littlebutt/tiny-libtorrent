CC = gcc
CFLAGS = -I include -g -Wall -O0
OS := $(shell uname)

ifeq ($(OS),Linux)
	LDFLAGS =
	TARGET = tiny-libtorrent
else ifeq ($(OS),Darwin)
	LDFLAGS =
	TARGET = tiny-libtorrent
else
	LDFLAGS = -lws2_32
	TARGET = tiny-libtorrent.exe
endif

SRCS = $(wildcard ./src/*.c ./src/*.S)
OBJS = $(patsubst ./src/%, ./build/%, $(SRCS:.c=.o))
OBJS := $(patsubst ./src/%, ./build/%, $(OBJS:.S=.o))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

./build/%.o: ./src/%.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
