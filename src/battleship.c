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
    game_ptr = malloc(sizeof *game_ptr);
    game_ptr->torpedeiro = calloc(TOR_N, sizeof *game_ptr->torpedeiro);
    game_ptr->porta_aviao = calloc(PAV_N, sizeof *game_ptr->porta_aviao);
    game_ptr->submarino = calloc(SUB_N, sizeof *game_ptr->submarino);
    game_ptr->couracado = calloc(COU_N, sizeof *game_ptr->couracado);

    game_ptr->total_ships = COU_N + SUB_N + PAV_N + TOR_N;

    for( int i = 0; i < ORDEM; i++ ){
        for( int j = 0; j < ORDEM; j++ ){
            game_ptr->grid[i][j] = water;
        }
    }

    deploy_units(torpedo);
    deploy_units(carrier);
    deploy_units(submarine);
    deploy_units(battleship);

    return;
}

void get_ship_specs(ship_specs_t *model, cell_t type){
    memset(model->proto.name, 0, BUFF_S);

    switch(type){
        case torpedo:
            model->proto.points = TOR_P;
            model->proto.width = TOR_W;
            model->proto.fleet_of = TOR_N;
            sprintf(model->proto.name, "Torpedeiro!");
            model->ship_ptr = game_ptr->torpedeiro;
            break;
        case carrier:
            model->proto.points = PAV_P;
            model->proto.width = PAV_W;
            model->proto.fleet_of = PAV_N;
            sprintf(model->proto.name, "Porta Avioes!");
            model->ship_ptr = game_ptr->porta_aviao;
            break;
        case submarine:
            model->proto.points = SUB_P;
            model->proto.width = SUB_W;
            model->proto.fleet_of = SUB_N;
            sprintf(model->proto.name, "Submarino!");
            model->ship_ptr = game_ptr->submarino;
            break;
        case battleship:
            model->proto.points = COU_P;
            model->proto.width = COU_W;
            model->proto.fleet_of = COU_N;
            sprintf(model->proto.name, "Couracado!");
            model->ship_ptr = game_ptr->couracado;
            break;
        default:
            break;
    }

    return;
}

void deploy_units(cell_t ship){
    struct timeval precision;
    ship_specs_t model;

    gettimeofday(&precision, NULL);
    srand((precision.tv_sec) + (precision.tv_usec));
    rand();

    get_ship_specs(&model, ship);

    for( int i = 0; i < model.proto.fleet_of; i++ ){
        model.ship_ptr[i].type = ship;
        model.ship_ptr[i].sink = false;
        model.ship_ptr[i].points = model.proto.points;
        model.ship_ptr[i].width = model.proto.width;
        model.ship_ptr[i].fleet_of = model.proto.fleet_of;
        strncpy(model.ship_ptr[i].name, model.proto.name, BUFF_S);
        model.ship_ptr[i].posicao = \
            place_on_grid(&model.ship_ptr[i], &(game_ptr->grid[0][0]));
    }

    return;
}

cell_t **place_on_grid(ship_t *ship, cell_t *grid){
    int c = 0;
    int jump;
    cell_t *try = NULL;
    cell_t **pos = calloc(ship->width, sizeof *pos);

    jump = ( rand() % 2 ) ? 1 : ORDEM;

    while( c < ship->width ){
        try = ( ! try ) ? grid + first_jump() : try + jump;

        if( *try != water ){
            c = 0;
            try = NULL;
            continue;
        }

        pos[c] = try;
        c++;
    }

    for( int i = 0; i < ship->width; i++ )
        *(pos[i]) = ship->type;

    return pos;
}

int first_jump(void){
    int x = ORDEM + rand() / (RAND_MAX / (0 - ORDEM + 1) + 1);
    int y = ORDEM + rand() / (RAND_MAX / (0 - ORDEM + 1) + 1);

    return x + y * ORDEM;
}


void game_cleanup(void){
    finish_units(torpedo);
    finish_units(carrier);
    finish_units(submarine);
    finish_units(battleship);

    free(game_ptr->torpedeiro);
    free(game_ptr->porta_aviao);
    free(game_ptr->submarino);
    free(game_ptr->couracado);

    game_ptr->torpedeiro = NULL;
    game_ptr->porta_aviao = NULL;
    game_ptr->submarino = NULL;
    game_ptr->couracado = NULL;

    free(game_ptr);
    game_ptr = NULL;

    return;
}

void finish_units(cell_t ship){
    ship_specs_t specs;

    get_ship_specs(&specs, ship);

    for( int i = 0; i < specs.proto.fleet_of; i++ ){
        specs.ship_ptr[i].type = ship;
        specs.ship_ptr[i].sink = false;
        specs.ship_ptr[i].points = 0;
        specs.ship_ptr[i].width = 0;
        specs.ship_ptr[i].fleet_of = 0;
        memset(specs.ship_ptr[i].name, 0, BUFF_S);
        free(specs.ship_ptr[i].posicao);
        specs.ship_ptr[i].posicao = NULL;
    }

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
    ship_t *head = NULL;

    for( int n_ship = 0; n_ship < ship->fleet_of; n_ship++ ){
        if( ship[n_ship].sink )
            continue;

        c = 0;
        pos = *ship[n_ship].posicao;
        head = ship + n_ship;

        for( int ship_w = 0; ship_w < ship->width; ship_w++ ){
            if( pos[ship_w] == hit )
                c++;
        }

        if( c == ship->width ){
            game_ptr->total_ships -= 1;
            head->sink = true;
            return head->points;
        }
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
