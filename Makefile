PKG_CONFIG=`pkg-config --cflags --libs gtk+-2.0`
CC=/usr/bin/gcc -g -Wall
LDFLAGS=$(PKG_CONFIG)
OUT=./bin

SRC = sample.c
sample: $(SRC)
	mkdir -p $(OUT)
	$(CC) $(SRC) -o $(OUT)/sample $(LDFLAGS)
