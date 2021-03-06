/*  Copyright (c) 2015 Carlos Millett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef H_BATTLESHIP
#define H_BATTLESHIP

#include <pthread.h>
#include <stdbool.h>

/*
 *  Global constants
 */
#define BUFF_S 256
#define NAME 15

#define ORDEM 100
#define MAX_PLAYERS 50
#define MAX_PLAYERS_SHOTS 20

#define COU_N 2
#define COU_W 5
#define COU_P 50

#define SUB_N 3
#define SUB_W 3
#define SUB_P 35

#define PAV_N 5
#define PAV_W 3
#define PAV_P 20

#define TOR_N 8
#define TOR_W 2
#define TOR_P 15

/*
 *  Data types
 */
enum cell{
    hit,
    water,
    torpedo,
    carrier,
    submarine,
    battleship
};
typedef enum cell cell_t;

struct ship{
    cell_t type;
    bool sink;
    int points;
    int width;
    char name[NAME];
    cell_t **posicao;
    struct ship *next;
};
typedef struct ship ship_t;

struct game{
    int total_ships;
    ship_t *torpedeiro;
    ship_t *porta_aviao;
    ship_t *submarino;
    ship_t *couracado;
    cell_t grid[ORDEM][ORDEM];
};
typedef struct game game_t;

struct player{
    int id;
    int tiros;
    int pontos;
    int socket;
    pthread_t thread;
    struct player *prev;
    struct player *next;
};
typedef struct player player_t;

/*
 *  External global variables
 */
extern pthread_barrier_t end_game_bar;
extern player_t *player_0;
extern game_t *game_ptr;

/*
 *  Functions declarations
 */
void game_setup(void);

ship_t *deploy_units(cell_t, int);

void place_on_grid(int, cell_t, cell_t**);

int first_jump(void);

void game_cleanup(void);

void finish_units(ship_t*);

player_t *game_fire(int, int, player_t*);

int is_sink(ship_t*);

player_t *next_player(player_t*);

void game_end(void);

void game_broadcast(char*);

#endif
