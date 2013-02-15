PKG_CONFIG=`pkg-config --cflags --libs gtk+-2.0`
CC=/usr/bin/gcc -g -Wall
LDFLAGS=$(PKG_CONFIG)
OUT=./out

all: readmaze sample

SRC = sample.c
sample: $(SRC) out
	$(CC) $(SRC) -o $(OUT)/sample $(LDFLAGS)

readmaze: readmaze.c out
	$(CC) circular_buffer.c readmaze.c -o $(OUT)/readmaze $(LDFLAGS)
PHONY += readmaze

out:
	mkdir -p $(OUT)

clean:
	rm -rf ./out

.PHONY: $(PHONY)

