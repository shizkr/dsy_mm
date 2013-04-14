PKG_CONFIG=`pkg-config --cflags --libs gtk+-2.0`
CC=/usr/bin/gcc -g -Wall
CINCLUDE = -Icore -Isimulation
CFLAGS = -DCONFIG_16X16
CFLAGS += -DDEBUG_MEMORY
#CFLAGS += -DDEBUG
# Limit memory size by reducing searching possible maze pathes
CFLAGS += -DCONFIG_PATH_LIMIT

LDFLAGS=$(PKG_CONFIG) $(CFLAGS) $(CINCLUDE)

all: gentable runsimul runsimul_gui
	@rm gentable
core: core_simul gentable

# core common files for simulator and real mouse
CORE_SRC = core/memory.c \
		   core/algo.c \
		   core/load_table.c \
		   core/diagonal.c

# it's for GUI simulation program only
GUI_SRC = simulation/maze.c \
		  simulation/draw_maze.c \
		  simulation/draw_mouse.c

# it's for non-GUI simulation program only
CORE_SIMUL = simulation/run_mouse.c \
			 simulation/simulator.c \
			 simulation/timer.c

# simulation for core algorithm
CORE_SIMUL_SRC = $(CORE_SIMUL) $(CORE_SRC)
core_simul: $(CORE_SIMUL_SRC)
	$(CC) $(CORE_SIMUL_SRC) -o simul $(LDFLAGS) -DDEBUG
PHONY += core_simul

# graphical full simulator
SIMUL_SRC = $(CORE_SIMUL) $(CORE_SRC) $(GUI_SRC)
runsimul_gui: $(SIMUL_SRC)
	$(CC) $(SIMUL_SRC) -o runsimul_gui $(LDFLAGS) -DMAZE_GUI
PHONY += runsimul_gui

# graphical full simulator
SIMUL_SRC = $(CORE_SIMUL) $(CORE_SRC) $(GUI_SRC)
runsimul: $(SIMUL_SRC)
	$(CC) $(SIMUL_SRC) -o runsimul $(LDFLAGS)
PHONY += runsimul

core/load_table.c: gentable
	./gentable

# turn weight generation tool
GENTABLE_SRC = utils/gentable.c
gentable:
	$(CC) $(GENTABLE_SRC) -o gentable $(LDFLAGS)
PHONY += gentable

test: test.c
	$(CC) test.c -o test $(LDFLAGS)

clean:
	@rm -f runsimul gentable
	@rm -f cscope*

.PHONY: $(PHONY)

