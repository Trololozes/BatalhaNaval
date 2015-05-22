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
#include <unistd.h>
#include <string.h>
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

    pthread_mutex_init(&game_msg_lock, NULL);
    pthread_cond_init(&game_msg_cond, NULL);

    for( int i = 0; i < ORDEM; i++ ){
        for( int j = 0; j < ORDEM; j++ ){
            game->grid[i][j] = water;
        }
    }

    deploy_units(torpedo, game);
    deploy_units(carrier, game);
    deploy_units(submarine, game);
    deploy_units(battleship, game);

    return game;
}

void deploy_units(cell_t ship, game_t *game){
    int points;
    int width;
    int size;
    navio_t *boat;

    switch(ship){
        case torpedo:
            points = 15;
            width = 2;
            size = TOR_N;
            boat = game->torpedeiro;
            break;
        case carrier:
            points = 20;
            width = 3;
            size = PAV_N;
            boat = game->porta_aviao;
            break;
        case submarine:
            points = 35;
            width = 3;
            size = SUB_N;
            boat = game->submarino;
            break;
        case battleship:
            points = 50;
            width = 5;
            size = COU_N;
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
    cell_t **pos = calloc(width+1, sizeof **pos);

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

    pos[width] = NULL;

    return pos;
}

game_t *game_cleanup(game_t *game){
    finish_units(torpedo, game);
    finish_units(carrier, game);
    finish_units(submarine, game);
    finish_units(battleship, game);

    free(game->torpedeiro);
    free(game->porta_aviao);
    free(game->submarino);
    free(game->couracado);

    game->torpedeiro = NULL;
    game->porta_aviao = NULL;
    game->submarino = NULL;
    game->couracado = NULL;

    free(game);

    pthread_mutex_destroy(&game_msg_lock);
    pthread_cond_destroy(&game_msg_cond);

    return NULL;
}

void finish_units(cell_t ship, game_t *game){
    int size;
    navio_t *boat;

    switch(ship){
        case torpedo:
            size = TOR_N;
            boat = game->torpedeiro;
            break;
        case carrier:
            size = PAV_N;
            boat = game->porta_aviao;
            break;
        case submarine:
            size = SUB_N;
            boat = game->submarino;
            break;
        case battleship:
            size = COU_N;
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

int game_fire(int x, int y, game_t *game){
    int i;
    int size;
    int points;
    char *output;
    cell_t target;
    cell_t *pos;
    navio_t *ship;

    target = game->grid[x][y];
    game->grid[x][y] = hit;

    switch(target){
        case torpedo:
            output = "Torpedeiro!";
            size = TOR_N;
            ship = game->torpedeiro;
            break;
        case carrier:
            output = "Porta-avioes!";
            size = PAV_N;
            ship = game->porta_aviao;
            break;
        case submarine:
            output = "Submarino!";
            size = SUB_N;
            ship = game->submarino;
            break;
        case battleship:
            output = "Couracado!";
            size = COU_N;
            ship = game->couracado;
            break;
        case water:
            return 0;
        case hit:
            return 0;
    }

    for( i = 0; i < size; i++ ){
        if( ship[i].sink )
            continue;
        for( pos = *(ship[i].posicao); pos != NULL; pos++ ){
            if( *pos != hit )
                break;
        }
        if( pos == NULL )
            break;
    }

    ship[i].sink = true;
    points = ship[i].points;
    ship[i].points = 0;

    return points;
}

void *broadcast_game(void *conn){
    int **socks = conn;

    pthread_mutex_lock(&game_msg_lock);
    pthread_cond_wait(&game_msg_cond, &game_msg_lock);

    for( int i = 0; i < MAX_PLAYERS; i++ )
        if( socks[i] != NULL )
            write(*socks[i], buffer, strlen(buffer));

    memset(buffer, 0, 256);
    pthread_mutex_unlock(&game_msg_lock);
    return 0;
}
