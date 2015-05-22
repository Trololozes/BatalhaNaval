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

#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "battleship.h"

static char buffer[256];
static pthread_mutex_t game_msg_lock;
static pthread_cond_t game_msg_cond;

game_t *game_setup(void){
    game_t *game = malloc(sizeof *game);
    game->torpedeiro = calloc(TOR_N, sizeof *game->torpedeiro);
    game->porta_aviao = calloc(PAV_N, sizeof *game->porta_aviao);
    game->submarino = calloc(SUB_N, sizeof *game->submarino);
    game->couracado = calloc(COU_N, sizeof *game->couracado);

    for( int i = 0; i < ORDEM; i++ ){
        for( int j = 0; j < ORDEM; j++ ){
            game->grid[i][j] = water;
        }
    }

    deploy_units(torpedo, TOR_N, game);
    deploy_units(carrier, PAV_N, game);
    deploy_units(submarine, SUB_N, game);
    deploy_units(battleship, COU_N, game);

    return game;
}

void deploy_units(cell_t ship, int size, game_t *game){
    int points;
    int width;
    navio_t *boat;

    switch(ship){
        case torpedo:
            points = 15;
            width = 2;
            boat = game->torpedeiro;
            break;
        case carrier:
            points = 20;
            width = 3;
            boat = game->porta_aviao;
            break;
        case submarine:
            points = 35;
            width = 3;
            boat = game->submarino;
            break;
        case battleship:
            points = 50;
            width = 5;
            boat = game->couracado;
            break;
        default:
            return;
    }

    for( int i = 0; i < size; i++ ){
        boat[i].points = points;
        boat[i].sink = false;
        boat[i].posicao = place_on_grid(ship, width, &(game->grid[0][0]));
    }
}

cell_t **place_on_grid(cell_t ship, int width, cell_t *grid){
    int x;
    int y;
    int c = 0;
    int jump;
    bool from_start = true;
    cell_t *try;
    cell_t **pos = calloc(width, sizeof **pos);

    srand(time(NULL));
    rand();

    jump = ( rand() % 2 ) ? 1 : ORDEM;

    while( c < width ){
        if( from_start ){
            x = rand() % 100;
            y = rand() % 100;
            try = grid + x + y;
            from_start = false;
        }
        else{
            try += jump;
        }

        if( *try != water ){
            c = 0;
            from_start = true;
            continue;
        }

        pos[c] = try;
        c++;
    }

    for( int i = 0; i < width; i++ )
        *(pos[i]) = ship;

    return pos;
}

game_t *game_cleanup(game_t *game){
    finish_units(torpedo, TOR_N, game);
    finish_units(carrier, PAV_N, game);
    finish_units(submarine, SUB_N, game);
    finish_units(battleship, COU_N, game);

    free(game->torpedeiro);
    free(game->porta_aviao);
    free(game->submarino);
    free(game->couracado);

    game->torpedeiro = NULL;
    game->porta_aviao = NULL;
    game->submarino = NULL;
    game->couracado = NULL;

    free(game);

    return NULL;
}

void finish_units(cell_t ship, int size, game_t *game){
    navio_t *boat;

    switch(ship){
        case torpedo:
            boat = game->torpedeiro;
            break;
        case carrier:
            boat = game->porta_aviao;
            break;
        case submarine:
            boat = game->submarino;
            break;
        case battleship:
            boat = game->couracado;
            break;
        default:
            return;
    }

    for( int i = 0; i < size; i++ ){
        boat[i].points = 0;;
        boat[i].sink = false;
        free(boat[i].posicao);
        boat[i].posicao = NULL;
    }
}

void *broadcast_game(void *conn){
    int **socks = conn;

    return 0;
}
