PKG_CONFIG=`pkg-config --cflags --libs gtk+-2.0`
CC=/usr/bin/gcc -g -Wall
CINCLUDE = -Icore -Isimulation
CFLAGS = -DDEBUG
LDFLAGS=$(PKG_CONFIG) $(CFLAGS) $(CINCLUDE)

all: simul gentable

core: core_simul gentable

CORE_SRC = core/bin_tree.c \
		   core/circular_buffer.c \
		   core/algo.c \
		   core/load_table.c \
		   core/diagonal.c

GUI_SRC = simulation/maze.c \
		  simulation/drawmaze.c \
		  simulation/drawmouse.c \
		  simulation/run_mouse.c

# simulation for core algorithm
CORE_SIMUL_SRC = simulation/simulator.c $(CORE_SRC)
core_simul: $(CORE_SIMUL_SRC)
	$(CC) $(CORE_SIMUL_SRC) -o simul $(LDFLAGS)
	@rm gentable
PHONY += core_simul

# graphical full simulator
SIMUL_SRC = simulation/simulator.c $(CORE_SRC) $(GUI_SRC)
simul: $(SIMUL_SRC)
	$(CC) $(SIMUL_SRC) -o simul $(LDFLAGS) -DMAZE_GUI
	@rm gentable
PHONY += simul

core/load_table.c: gentable
	./gentable

# turn weight generation tool
GENTABLE_SRC = utils/gentable.c
gentable:
	$(CC) $(GENTABLE_SRC) -o gentable $(LDFLAGS)
PHONY += gentable

clean:
	@rm -f simul gentable
	@rm -f cscope*

.PHONY: $(PHONY)

