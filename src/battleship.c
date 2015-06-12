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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "battleship.h"

/*
 *  Functions definitions
 */
void game_setup(void){
    struct timeval precision;

    gettimeofday(&precision, NULL);
    srand((precision.tv_sec) + (precision.tv_usec));
    rand();

    game_ptr = malloc(sizeof *game_ptr);
    game_ptr->total_ships = COU_N + SUB_N + PAV_N + TOR_N;

    for( int i = 0; i < ORDEM; i++ ){
        for( int j = 0; j < ORDEM; j++ ){
            game_ptr->grid[i][j] = water;
        }
    }

    game_ptr->torpedeiro = deploy_units(torpedo, TOR_N);
    game_ptr->porta_aviao = deploy_units(carrier, PAV_N);
    game_ptr->submarino = deploy_units(submarine, SUB_N);
    game_ptr->couracado = deploy_units(battleship, COU_N);

    return;
}

ship_t *deploy_units(cell_t ship_model, int n){
    ship_t *real;

    if( ! n ){
        return NULL;
    }

    real = malloc(sizeof *real);
    memset(real->name, 0, NAME);

    real->type = ship_model;

    switch(ship_model){
        case torpedo:
            real->points = TOR_P;
            real->width = TOR_W;
            sprintf(real->name, "Torpedeiro!");
            break;
        case carrier:
            real->points = PAV_P;
            real->width = PAV_W;
            sprintf(real->name, "Porta Avioes!");
            break;
        case submarine:
            real->points = SUB_P;
            real->width = SUB_W;
            sprintf(real->name, "Submarino!");
            break;
        case battleship:
            real->points = COU_P;
            real->width = COU_W;
            sprintf(real->name, "Couracado!");
            break;
        default:
            break;
    }

    real->posicao = calloc(real->width, sizeof *real->posicao);
    place_on_grid(real->width, real->type, real->posicao);
    real->next = deploy_units(ship_model, n-1);

    return real;
}

void place_on_grid(int width, cell_t type, cell_t **pos){
    int c = 0;
    int jump;
    cell_t *try = NULL;
    cell_t *grid = &game_ptr->grid[0][0];

    jump = ( rand() % 2 ) ? 1 : ORDEM;

    while( c < width ){
        try = ( ! try ) ? grid + first_jump() : try + jump;

        if( *try != water ){
            c = 0;
            try = NULL;
            continue;
        }

        pos[c] = try;
        c++;
    }

    for( int i = 0; i < width; i++ )
        *(pos[i]) = type;

    return;
}

int first_jump(void){
    int x = ORDEM + rand() / (RAND_MAX / (0 - ORDEM + 1) + 1);
    int y = ORDEM + rand() / (RAND_MAX / (0 - ORDEM + 1) + 1);

    return x + y * ORDEM;
}


void game_cleanup(void){
    finish_units(game_ptr->torpedeiro);
    finish_units(game_ptr->porta_aviao);
    finish_units(game_ptr->submarino);
    finish_units(game_ptr->couracado);

    free(game_ptr);
    game_ptr = NULL;

    return;
}

void finish_units(ship_t *ship){
    if( ! ship ){
        return;
    }

    ship->type = water;
    ship->sink = false;
    ship->points = 0;
    ship->width = 0;
    memset(ship->name, 0, NAME);
    free(ship->posicao);
    ship->posicao = NULL;

    finish_units(ship->next);

    free(ship);

    return;
}

player_t *game_fire(int x, int y, player_t *player){
    int pontos;
    char buffer[BUFF_S];
    cell_t target;
    player_t *next = NULL;
    ship_t *ship = NULL;

    memset(buffer, 0, BUFF_S);

    target = game_ptr->grid[x][y];
    game_ptr->grid[x][y] = hit;
    player->tiros--;

    sprintf(buffer, "-- [%dx%d] -> ", x, y);

    switch(target){
        case hit:
            strncat(buffer, "Ja dispararam aqui... entao, ", BUFF_S);
        case water:
            strncat(buffer, "Splash", BUFF_S);
            break;
        case torpedo:
            ship = game_ptr->torpedeiro;
            break;
        case carrier:
            ship = game_ptr->porta_aviao;
            break;
        case submarine:
            ship = game_ptr->submarino;
            break;
        case battleship:
            ship = game_ptr->couracado;
            break;
    }

    if( ship ){
        pontos = is_sink(ship);
        strncat(buffer, ship->name, BUFF_S);
        strncat(buffer, (pontos) ? " - Afundado!" : " - Atingido!", BUFF_S);
        player->pontos += pontos;
        next = player;
    }

    if( ! next )
        next = next_player(player);

    strncat(buffer, "\n", BUFF_S);
    game_broadcast(buffer);

    if( ! ( game_ptr->total_ships && next ) ){
        game_end();
        return NULL;
    }

    memset(buffer, 0, BUFF_S);
    sprintf(buffer, "== Player#%d (Pontuacao: %d | Tiros: %d)\n",\
            next->id, next->pontos, next->tiros);
    game_broadcast(buffer);

    return next;
}

int is_sink(ship_t *ship){
    int c;
    cell_t *pos = NULL;

    while( ship ){
        if( ship->sink )
            continue;

        c = 0;
        pos = *ship->posicao;

        for( int ship_w = 0; ship_w < ship->width; ship_w++ ){
            if( pos[ship_w] == hit )
                c++;
        }

        if( c == ship->width ){
            game_ptr->total_ships -= 1;
            ship->sink = true;
            return ship->points;
        }

        ship = ship->next;
    }

    return 0;
}

player_t *next_player(player_t *current){
    player_t *next = current->next;

    while( next != current ){
        if( next->tiros )
            return next;

        next = next->next;
    }

    if( next->tiros )
        return next;

    return NULL;
}

void game_end(void){
    char buffer[BUFF_S];

    for( player_t *play = player_0->next; play != player_0; play = play->next ){
        memset(buffer, 0, BUFF_S);

        sprintf(buffer, "-- Player#%d -> %d pontos\n", play->id, play->pontos);
        game_broadcast(buffer);
    }

    pthread_barrier_wait(&end_game_bar);
    return;
}

void game_broadcast(char *msg){
    for( player_t *play = player_0->next; play != player_0; play = play->next )
        send(play->socket, msg, strlen(msg), 0);

    return;
}
