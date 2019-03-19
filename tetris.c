#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>

#if 0

	Pieces:
	 ___   _       _   _   _       _
	|_|_| |_|     |_| |_| |_|_   _|_|   _ _ _
	|_|_| |_|_   _|_| |_| |_|_| |_|_|  |_|_|_|
              |_|_| |_|_| |_|   |_| |_|      |_|
                          |_|

	  0 1 2 3
	0 X X X X  A piece is defined by its
	1 X X X X  positioning in a 4 x 4 grid
	2 X X X X 
	3 X X X X  i.e., [ [0, 3], [1, 3], [2, 3], [3, 3] ]

#endif

#define BOARD_HEIGHT 24
#define BOARD_WIDTH 10

int DOG_XY[2][4][2] = {
				{ {0, 1}, {0, 2}, {1, 2}, {1, 3} },
				{ {0, 3}, {1, 3}, {1, 2}, {2, 2} }
		   };

int DOG_LEN = 2;

int LEFT_DOG_XY[2][4][2] = {
				{ {1, 1}, {1, 2}, {0, 2}, {0, 3} },
				{ {0, 2}, {1, 2}, {1, 3}, {2, 3} } 
		};

int LEFT_DOG_LEN = 2;

int T_XY[4][4][2] = {
				{ {0, 3}, {1, 3}, {2, 3}, {1, 2} },
				{ {0, 2}, {1, 2}, {1, 1}, {1, 3} },
				{ {0, 2}, {1, 2}, {2, 2}, {1, 3} },
				{ {1, 1}, {1, 2}, {1, 3}, {2, 2} }
		};

int T_LEN = 4;

int SQUARE_XY[1][4][2] = {		 
				{ {0, 3}, {1, 3}, {0, 2}, {1, 2} } 
		            };

int SQUARE_LEN = 1;


int FORW_L_XY[4][4][2] = {
				{ {0, 1}, {0, 2}, {0, 3}, {1, 3} },
				{ {0, 3}, {1, 3}, {2, 3}, {2, 2} },
				{ {1, 3}, {1, 2}, {1, 1}, {0, 1} },
				{ {0, 3}, {0, 2}, {1, 2}, {2, 2} } 				
		            };

int FORW_L_LEN = 4;


int BACK_L_XY[4][4][2] = {
				{ {0, 3}, {1, 3}, {1, 2}, {1, 1} },
				{ {0, 2}, {1, 2}, {2, 2}, {2, 3} },
				{ {0, 3}, {0, 2}, {0, 1}, {1, 1} },
				{ {0, 1}, {0, 2}, {1, 2}, {2, 2} }
			    };

int BACK_L_LEN = 4;


int LONG_XY[2][4][2] = {
				{ {0, 0}, {0, 1}, {0, 2}, {0, 3} },
				{ {0, 0}, {1, 0}, {2, 0}, {3, 0} }
			  };

int LONG_LEN = 2;


static int* PIECE_CONFIGURATIONS[7] = {&SQUARE_XY[0][0][0], &FORW_L_XY[0][0][0], &BACK_L_XY[0][0][0], &LONG_XY[0][0][0], &DOG_XY[0][0][0], &LEFT_DOG_XY[0][0][0], &T_XY[0][0][0]};
static int  PIECE_CONFIG_SIZES[7] = {1, 4, 4, 2, 2, 2, 4};
int  NUM_CONFIGURATIONS = 7;
int PIECE_COLORS[7] = {1, 2, 3, 4, 5, 6, 7}; 

struct block{
	int color;
	int present;
};

struct tetris_piece{
	int moving;
	
	int x_pos;	  //Position of bottom left corner
	int y_pos;	  //Position of botthom left corner'

	int *positions;
	int cur_position; 
	int num_positions;

	int color;
};

struct block blocks[BOARD_HEIGHT][BOARD_WIDTH]; 
    
struct tetris_piece *cur_piece;

char game_move = 0;
int game_end = 0;

int max_y;
int max_x;

void draw_debug(struct tetris_piece *draw);
struct tetris_piece * get_new_piece();
void draw_blocks();
int piece_in_spot(int y, int x);
void move_piece_down();
void* get_move(void* args);
void move_piece_right();
void move_piece_left();
void* game_loop(void* args);
int piece_stopped();
void piece_into_blocks();
void rotate_piece();
void drop_piece();
void clear_lines();

int main()
{
	printf("Tetris? Tetris!\n");

	WINDOW *stdscr = initscr();
  	getmaxyx(stdscr, max_y, max_x);

	for(int i = 0; i < BOARD_HEIGHT; i++)
		for(int j = 0; j < BOARD_WIDTH; j++){
			blocks[i][j].present = 0;
		}	

	pthread_t move_getter;
	pthread_t game_runner;

	initscr();
	
	noecho();
	curs_set(FALSE);

	cur_piece = get_new_piece();

	pthread_create(&move_getter, NULL, get_move, NULL);
	pthread_create(&game_runner, NULL, game_loop, NULL);
	
	pthread_join(move_getter, NULL);
	pthread_join(game_runner, NULL);
	
	endwin();

	return 0;
}

void test_loss()
{
	for(int i = 0; i < BOARD_WIDTH; i++)
	{
		if(blocks[0][i].present)
			game_end = 1;
	}
}

void* get_move(void* args)
{
	char tmp;

	while(!game_end){

		tmp = getch();

		//Arrow keys
		if(tmp == '\033') {
			getch();			
			game_move = getch(); // A = up, B = down, C = right, D = left
		}
		else if(tmp == ' ') {
			game_move = tmp;
		}

		switch(game_move){
			case 'C':
				move_piece_right();
				draw_blocks();
				break;
			case 'D':
				move_piece_left();
				draw_blocks();
				break;
			case 'A':
				rotate_piece();
				draw_blocks();
				break;
			case ' ':
				drop_piece();
				draw_blocks();
				break;
		}
	}

	return NULL;
}

void* game_loop(void* args)
{
	struct timespec timer;

	while(!game_end){
	
		draw_blocks();

		timer.tv_sec = 0;
		timer.tv_nsec = 400000000;

		nanosleep(&timer, NULL);
		
		if(piece_stopped()){
			piece_into_blocks();
			free(cur_piece);
			clear_lines();
			test_loss();
			cur_piece = get_new_piece();
		}

		move_piece_down();
	}

	return NULL;
}

void clear_lines()
{
	
	int num_present;

	for(int i = 0; i < BOARD_HEIGHT; i++){
		
		num_present = 0;
		
		for(int j = 0; j < BOARD_WIDTH; j++){
			if(blocks[i][j].present)
				num_present++;
		}

		if(num_present == BOARD_WIDTH){
			for(int j = 0; j < BOARD_WIDTH; j++){
				blocks[i][j].present = 0;
			}
			for(int k = i; k > 0; k--){
				for(int j = 0; j < BOARD_WIDTH; j++){
					blocks[k][j] = blocks[k-1][j];
					blocks[k-1][j].present = 0; 
				}
			}
		}
	}
}

void drop_piece()
{
	while(!piece_stopped())
		move_piece_down();	
	piece_into_blocks();
	clear_lines();
	free(cur_piece);
	cur_piece = get_new_piece();
}

//Uses Wall Kicking
void rotate_piece()
{
	int new_position_num = (cur_piece->cur_position + 1) % (cur_piece->num_positions);
	
	int* new_position = &cur_piece->positions[8 * new_position_num];	

	int y = cur_piece->y_pos;
	int x = cur_piece->x_pos;

	int bad = 0;
	
	for(int i = 0; i < 4; i++){
		if(blocks[new_position[2*i + 1] + y][new_position[2*i + 0] + x].present
			|| new_position[2*i + 0] + x >= BOARD_WIDTH
			|| new_position[2*i + 0] + x < 0)
			bad = 1;
	}
	
	if(!bad){
		printf("Fine");
		cur_piece->cur_position = new_position_num;
		return;
	}
	bad = 0;

	for(int i = 0; i < 4; i++){
		if(blocks[new_position[2*i + 1] + y][new_position[2*i + 0] + x + 1].present
			|| new_position[2*i + 0] + x + 1 >= BOARD_WIDTH
			|| new_position[2*i + 0] + x + 1 < 0)
			bad = 1;
	}

	if(!bad){
		cur_piece->cur_position = new_position_num;
		cur_piece->x_pos++;
		return;
	}
	
	bad = 0;
	
	for(int i = 0; i < 4; i++){
		if(blocks[new_position[2*i + 1] + y][new_position[2*i + 0] + x - 1].present
			|| new_position[2*i + 0] + x - 1 < 0
			|| new_position[2*i + 0] + x - 1 >= BOARD_WIDTH)
			bad = 1;
	}

	if(!bad){
		cur_piece->cur_position = new_position_num;
		cur_piece->x_pos--;
		return;
	}
}
void move_piece_down()
{
	cur_piece->y_pos += 1;
}

void move_piece_right()
{
	
	int* positions = &cur_piece->positions[8 * cur_piece->cur_position];
        int y = cur_piece->y_pos;
        int x = cur_piece->x_pos;	

	for(int i = 0; i < 4; i++)	
		if(x + positions[2*i + 0] + 1 == BOARD_WIDTH || blocks[positions[2*i + 1] + y][positions[2*i + 0] + x + 1].present)
			return;

	cur_piece->x_pos++;
}

void move_piece_left()
{
	int* positions = &cur_piece->positions[8 * cur_piece->cur_position];
        int y = cur_piece->y_pos;
        int x = cur_piece->x_pos;

	for(int i = 0; i < 4; i++)
		if(x + positions[2*i + 0] - 1 < 0 || blocks[positions[2*i + 1] + y][positions[2*i + 0] + x - 1].present)
			return;
	
	cur_piece->x_pos--;
}

int piece_stopped()
{
	int* positions = &cur_piece->positions[8 * cur_piece->cur_position];
	int y = cur_piece->y_pos;
	int x = cur_piece->x_pos;

	for(int i = 0; i < 4; i++){
		if(positions[2*i + 1] + y + 1 == BOARD_HEIGHT || 
			blocks[positions[2*i + 1] + y + 1][positions[2*i + 0] + x].present)
			
			return 1;
	}
	
	return 0;
}

//Turn a piece into blocks
void piece_into_blocks()
{
        int y = cur_piece->y_pos;
        int x = cur_piece->x_pos;

   	int* positions = &cur_piece->positions[8 * cur_piece->cur_position];
	
	for(int i = 0; i < 4; i++)
	{
		blocks[positions[2*i+1]+y][positions[2*i]+x].present = 1;
		blocks[positions[2*i+1]+y][positions[2*i]+x].color = cur_piece->color;
	}
}

void draw_blocks()
{
	clear();

	start_color();

        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        init_pair(2, COLOR_WHITE, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
	init_pair(5, COLOR_RED, COLOR_BLACK);
	init_pair(6, COLOR_GREEN, COLOR_BLACK);
	init_pair(7, COLOR_MAGENTA, COLOR_BLACK);	

	attron(COLOR_PAIR(2));
	for(int i = 0; i < BOARD_WIDTH*3 + 1; i++)
		mvprintw(BOARD_HEIGHT-1, max_x/2 + i - 3*BOARD_WIDTH/2, "_");

	for(int i = 0; i < BOARD_HEIGHT; i++){
		attron(COLOR_PAIR(2));
		mvprintw(i, max_x / 2 - 3*BOARD_WIDTH/2 - 1, "|");
		mvprintw(i, max_x / 2 + 3*BOARD_WIDTH/2, "|");		

		for(int j = 0; j < BOARD_WIDTH; j++){

			//blocks[i][j].color = 1;
			//blocks[i][j].present = rand() % 2;

			if(blocks[i][j].present || piece_in_spot(i, j)){
				
				if(blocks[i][j].present)
					attron(COLOR_PAIR(blocks[i][j].color));
				else
					attron(COLOR_PAIR(cur_piece->color));

				mvprintw(i, max_x / 2 + 3*j - 3*BOARD_WIDTH/2, "|");
				mvprintw(i, max_x / 2 + 3*j + 1 - 3*BOARD_WIDTH/2, "_");
				mvprintw(i, max_x / 2 + 3*j + 2 - 3*BOARD_WIDTH/2, "|");
				
				if(i != 0)
					mvprintw(i-1, max_x /2 + 3*j + 1 - 3*BOARD_WIDTH/2, "_");
			}
		}
	}

	refresh();
}

int piece_in_spot(int y, int x)
{
	int* positions = &cur_piece->positions[8 * cur_piece->cur_position];
	
	for(int i = 0; i < 4; i++){
		if(positions[2*i + 0] + cur_piece->x_pos == x 
			&& positions[2*i + 1] + cur_piece->y_pos == y)
			return 1;
	}
	
	return 0;
}

struct tetris_piece * get_new_piece()
{
	struct tetris_piece *new_piece = (struct tetris_piece *) malloc(sizeof(struct tetris_piece));
	
	if(!new_piece){
		printf("Malloc failure!?\n");
		exit(1);
	}

	int num = rand() % NUM_CONFIGURATIONS;
	
	new_piece->x_pos = 0;
	new_piece->y_pos = 0;
	
	new_piece->positions = PIECE_CONFIGURATIONS[num];
	new_piece->cur_position = 0;
	new_piece->num_positions = PIECE_CONFIG_SIZES[num];
	
	new_piece->color = PIECE_COLORS[num];

	return new_piece;
}
  
void draw_debug(struct tetris_piece *draw)
{
	int* positioning = &draw->positions[8 * draw->cur_position];

	int grid[4][4] = {{0}};

	#if 0
	for(int i = 0; i < 4; i++){
                printf("{");
                for(int j = 0; j < 2; j++){
                        printf("%i", positioning[2*i + j]);
                        if(j == 0)
                                printf(", ");
                }
                printf("}\n");
        }
	#endif
	
	for(int i = 0; i < 4; i++)
	{
		grid[positioning[2*i + 1]][positioning[2*i + 0]] = 1;
	}
	
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			printf("%c", grid[i][j] == 0 ? 'O' : 'X');
		}
		printf("\n");
	}	
}
