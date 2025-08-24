CC = gcc
CFLAGS = -Wall

SRC = $(wildcard internet_socket/*/*.c unix_socket/*/*.c)
BINS = $(patsubst %.c,build/%,$(SRC))

all: $(BINS)

# Rule build: build/foo/bar.c -> build/foo/bar
build/%: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build
