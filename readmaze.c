#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAZEMAX 256
/* Input maze from file */
char maze_file[MAZEMAX];
/* Memorize maze information */
char maze_search[MAZEMAX];

char contour_map[MAZEMAX];

#define NI	0
#define EI	1
#define SI	2
#define WI	3

#define NORTH	1
#define EAST	2
#define SOUTH	4
#define WEST	8

#define MAX_X	15
#define MAX_Y	15

#define wall_bit(direc) (0x01 << (direc))

#define pos_x(index)	((index>>4)&0xf)
#define pos_y(index)	(index&0xf)
#define get_index(x,y)	(((x)<<4)|(y))

int load_maze(char *filename, char *maze) {

    FILE* fd = NULL;
    memset(maze,0,sizeof(maze));

    fd = fopen(filename,"r+");

    if(NULL == fd) {
        printf("\n fopen() Error!!!\n");
        return 1;
    }

    if(MAZEMAX != fread(maze,1,MAZEMAX,fd)) {
        printf("\n fread() failed\n");
        return 1;
    }

    fclose(fd);
    return 0;
}

/* Initialize maze memory to have known maze information */
void initialize_maze(char *maze) {
	int x, y;

	for(x=0; x<=MAX_X; x++) {
		for(y=0; y<=MAX_Y; y++) {
			if(x==0)
				maze[get_index(x, y)] |= WEST;
			if(y==0)
				maze[get_index(x, y)] |= SOUTH;
			if(x==MAX_X)
				maze[get_index(x, y)] |= EAST;
			if(y==MAX_Y)
				maze[get_index(x, y)] |= NORTH;

		}
	}

	maze[get_index(0, 0)] |= EAST;
}

/* Draw maze information or contour map.
 * Map must be 256 bytes.
 */
void print_map(char *map)
{
	int x, y;

	for(y=MAX_Y; y>=0; y--) {
		for(x=0; x<=MAX_X; x++) {
			printf("%02X", map[get_index(x, y)]);
		}
		printf("\n");
	}
	printf("\n");
}

void draw_contour(char *maze, char *map) {
	int contour_lvl = 1;
	int x, y;
   	/* Use 2 array set to draw contour level */
	int array_index = 0, array_next_index;
	int	curr_i, next_i;
	char array[2][MAZEMAX];

	/* Uninitialized contour map */
    memset(map, 0, MAZEMAX);

	/* Seed value 1 as a goal */
	map[get_index(7, 7)] = contour_lvl;
	map[get_index(8, 7)] = contour_lvl;
	map[get_index(7, 8)] = contour_lvl;
	map[get_index(8, 8)] = contour_lvl;

	/* Add list of same level contour value */
	memset(&array[0][0], 0, MAZEMAX);
	array[0][0] = get_index(7, 7);
	array[0][1] = get_index(8, 7);
	array[0][2] = get_index(7, 8);
	array[0][3] = get_index(8, 8);

	while(1) {
		if (array[array_index][0] == 0)
			break;
		contour_lvl++;
		curr_i = next_i = 0;
		array_next_index = ((array_index + 1) & 1);
		memset(&array[array_next_index][0], 0, MAZEMAX);
		while(array[array_index][curr_i]) {
			x = pos_x(array[array_index][curr_i]);
			y = pos_y(array[array_index][curr_i]);

			/* Check north wall */
			if(!(maze[get_index(x, y)] & NORTH) &&
				(map[get_index(x, y+1)] == 0)) {
				map[get_index(x, y+1)] = contour_lvl;
				array[array_next_index][next_i++] = get_index(x, y+1);
			}
			/* Check east wall */
			if(!(maze[get_index(x, y)] & EAST) &&
				(map[get_index(x+1, y)] == 0)) {
				map[get_index(x+1, y)] = contour_lvl;
				array[array_next_index][next_i++] = get_index(x+1, y);
			}
			/* Check south wall */
			if(!(maze[get_index(x, y)] & SOUTH) &&
				(map[get_index(x, y-1)] == 0)) {
				map[get_index(x, y-1)] = contour_lvl;
				array[array_next_index][next_i++] = get_index(x, y-1);
			}
			/* Check west wall */
			if(!(maze[get_index(x, y)] & WEST) &&
				(map[get_index(x-1, y)] == 0)) {
				map[get_index(x-1, y)] = contour_lvl;
				array[array_next_index][next_i++] = get_index(x-1, y);
			}
			curr_i++;
		}
		array_index = array_next_index;
#ifdef DEBUG
		print_map(map);
		sleep(1);
#endif
	}

}

int main(int argc, char* argv[])
{
	printf("%s\n", argv[0]);
	printf("%s\n", argv[1]);
	load_maze(argv[1], maze_file);

	printf("load maze\n");
	print_map(maze_file);

	printf("Initialized maze\n");
	initialize_maze(maze_search);
	print_map(maze_search);

	//draw_contour(maze_search, contour_map);
	draw_contour(maze_file, contour_map);
	exit(EXIT_SUCCESS);
}
