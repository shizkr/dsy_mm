PKG_CONFIG=`pkg-config --cflags --libs gtk+-2.0`
CC=/usr/bin/gcc -g -Wall
CINCLUDE = -Icore
CFLAGS =
LDFLAGS=$(PKG_CONFIG) $(CFLAGS) $(CINCLUDE)

all: simul sample gentable

CORE_SRC = core/bin_tree.c \
		   core/circular_buffer.c \
		   core/algo.c \
		   core/load_table.c \
		   core/diagonal.c

# simulation for core algorithm
SIMUL_SRC = simulation/simulator.c $(CORE_SRC)
simul: $(SIMUL_SRC)
	$(CC) $(SIMUL_SRC) -o simul $(LDFLAGS)
	@rm gentable
PHONY += simul

# graphical full simulator
SRC = simulation/sample.c
sample: $(SRC)
	$(CC) $(SRC) -o sample $(LDFLAGS)

core/load_table.c: gentable
	./gentable

# turn weight generation tool
GENTABLE_SRC = utils/gentable.c
gentable:
	$(CC) $(GENTABLE_SRC) -o gentable $(LDFLAGS)
PHONY += gentable

clean:
	@rm -f simul sample gentable
	@rm -f cscope*

.PHONY: $(PHONY)

