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

#define MAX_PLAYERS 50
#define ORDEM 100

#define COU_N 2
#define SUB_N 3
#define PAV_N 5
#define TOR_N 8

#define COU_W 5
#define SUB_W 3
#define PAV_W 3
#define TOR_W 2

/*  data types  */
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
    int points;
    bool sink;
    cell_t **posicao;
};
typedef struct ship ship_t;

struct game{
    ship_t *torpedeiro;
    ship_t *porta_aviao;
    ship_t *submarino;
    ship_t *couracado;
    cell_t grid[ORDEM][ORDEM];
};
typedef struct game game_t;

struct player{
    int id;
    int pontos;
    int socket;
    pthread_t thread;
    game_t *game;
    struct player *prev;
    struct player *next;
};
typedef struct player player_t;

/*  global variables    */
extern pthread_barrier_t end_game_bar;
extern player_t *all_players;

/*  function declarations   */
game_t *game_setup(void);

void deploy_units(cell_t, game_t*);

cell_t **place_on_grid(cell_t, int, cell_t*);

game_t *game_cleanup(game_t*);

void finish_units(cell_t, game_t*);

void game_fire(int, int, player_t*);

int is_sink(ship_t*, int, int);

void game_end(void);

void game_broadcast(char*);

#endif
