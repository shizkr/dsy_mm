PKG_CONFIG=`pkg-config --cflags --libs gtk+-2.0`
CC=/usr/bin/gcc -g -Wall
CINCLUDE = -Icore
CFLAGS = -DDEBUG
LDFLAGS=$(PKG_CONFIG) $(CFLAGS) $(CINCLUDE)
OUT=./out

all: simulator sample

CORE_SRC = core/bin_tree.c \
		   core/circular_buffer.c \
		   core/algo.c

SRC = sample.c
sample: $(SRC) out
	$(CC) $(SRC) -o $(OUT)/sample $(LDFLAGS)

simulator: $(CORE_SRC) simulator.c out
	$(CC) $(CORE_SRC) simulator.c -o $(OUT)/simulator $(LDFLAGS)
PHONY += simulator

out:
	mkdir -p $(OUT)

clean:
	rm -rf ./out
	rm cscope*

.PHONY: $(PHONY)

