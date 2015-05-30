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
 *  Global variable within this file
 */
static int deployed_ships = COU_N + SUB_N + PAV_N + TOR_N;

/*
 *  Functions definitions
 */
void game_setup(void){
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

    game_ptr = game;

    deploy_units(torpedo);
    deploy_units(carrier);
    deploy_units(submarine);
    deploy_units(battleship);
}

void get_ship_specs(ship_specs_t *specs, cell_t cell){
    memset(specs->name, 0, BUFF_S);

    switch(cell){
        case torpedo:
            specs->points = TOR_P;
            specs->width = TOR_W;
            specs->fleet_of = TOR_N;
            specs->ship_ptr = game_ptr->torpedeiro;
            sprintf(specs->name, "Torpedeiro!");
            break;
        case carrier:
            specs->points = PAV_P;
            specs->width = PAV_W;
            specs->fleet_of = PAV_N;
            specs->ship_ptr = game_ptr->porta_aviao;
            sprintf(specs->name, "Porta Avioes!");
            break;
        case submarine:
            specs->points = SUB_P;
            specs->width = SUB_W;
            specs->fleet_of = SUB_N;
            specs->ship_ptr = game_ptr->submarino;
            sprintf(specs->name, "Submarino!");
            break;
        case battleship:
            specs->points = COU_P;
            specs->width = COU_W;
            specs->fleet_of = COU_N;
            specs->ship_ptr = game_ptr->couracado;
            sprintf(specs->name, "Couracado!");
            break;
        default:
            specs->points = 0;;
            specs->width = 0;;
            specs->fleet_of = 0;;
            specs->ship_ptr = NULL;
            break;
    }
}

void deploy_units(cell_t ship){
    struct timeval precision;
    ship_specs_t specs;

    gettimeofday(&precision, NULL);
    srand((precision.tv_sec) + (precision.tv_usec));
    rand();

    get_ship_specs(&specs, ship);

    for( int i = 0; i < specs.fleet_of; i++ ){
        specs.ship_ptr[i].points = specs.points;
        specs.ship_ptr[i].sink = false;
        specs.ship_ptr[i].posicao = \
            place_on_grid(ship, specs.width, &(game_ptr->grid[0][0]));
    }
}

cell_t **place_on_grid(cell_t ship, int width, cell_t *grid){
    int x;
    int y;
    int c = 0;
    int jump;
    bool from_start = true;
    cell_t *try;
    cell_t **pos = calloc(width, sizeof *pos);

    jump = ( rand() % 2 ) ? 1 : ORDEM;

    while( c < width ){
        if( from_start ){
            x = ORDEM + rand() / (RAND_MAX / (0 - ORDEM + 1) + 1);
            y = ORDEM + rand() / (RAND_MAX / (0 - ORDEM + 1) + 1);
            try = grid + x + y * ORDEM;
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
}

void finish_units(cell_t ship){
    ship_specs_t specs;

    get_ship_specs(&specs, ship);

    for( int i = 0; i < specs.fleet_of; i++ ){
        specs.ship_ptr[i].points = 0;;
        specs.ship_ptr[i].sink = false;
        free(specs.ship_ptr[i].posicao);
        specs.ship_ptr[i].posicao = NULL;
    }
}

void game_fire(int x, int y, player_t *player){
    int points;
    int kill_switch = 0;
    char buffer[BUFF_S];
    char n_round[BUFF_S];
    cell_t target;
    player_t *next;
    ship_specs_t specs;

    memset(buffer, 0, BUFF_S);
    memset(n_round, 0, BUFF_S);

    target = game_ptr->grid[x][y];
    game_ptr->grid[x][y] = hit;
    player->tiros--;

    sprintf(buffer, "-- [%dx%d] -> ", x, y);

    get_ship_specs(&specs, target);

    switch(target){
        case hit:
            strncat(buffer, "Ja dispararam aqui... entao, ", BUFF_S);

        case water:
            strncat(buffer, "Splash", BUFF_S);

            do{
                if( player->next->id == 0 ){
                    next = player->next->next;
                    kill_switch++;
                }
                else{
                    next = player->next;
                }
            }while( next->tiros == 0 && kill_switch < 2 );
            break;

        default:
            if( (points = is_sink(specs.ship_ptr, specs.fleet_of, specs.width)) )
                deployed_ships--;

            strncat(buffer, specs.name, BUFF_S);
            strncat(buffer, (points) ? " - Afundado!" : " - Atingido!", BUFF_S);
            player->pontos += points;
            next = player;
            break;
    }

    strncat(buffer, "\n", BUFF_S);
    game_broadcast(buffer);

    if( ! deployed_ships || kill_switch == 2 ){
        game_end();
        pthread_barrier_wait(&end_game_bar);

        return;
    }

    sprintf(n_round, "== Player#%d (Pontuacao: %d | Tiros: %d)\n",\
            next->id, next->pontos, next->tiros);
    game_broadcast(n_round);

    return;
}

int is_sink(ship_t *ship, int size, int width){
    int c;
    int points = 0;
    cell_t *pos = NULL;
    ship_t *head = NULL;

    for( int n_ship = 0; n_ship < size; n_ship++ ){
        if( ship[n_ship].sink )
            continue;

        c = 0;

        pos = *ship[n_ship].posicao;
        head = ship + n_ship;

        for( int ship_w = 0; ship_w < width; ship_w++ ){
            if( pos[ship_w] == hit )
                c++;
        }

        if( c == width ){
            head->sink = true;
            points = head->points;
            break;
        }
    }

    return points;
}

void game_end(void){
    char buffer[BUFF_S];
    char msg[BUFF_S];

    memset(msg, 0, BUFF_S);

    for( player_t *play = all_players->next; play->id != 0; play = play->next ){
        memset(buffer, 0, BUFF_S);

        sprintf(buffer, "-- Player#%d -> %d pontos\n", play->id, play->pontos);
        strncat(msg, buffer, BUFF_S);
    }

    game_broadcast(msg);
}

void game_broadcast(char *msg){
    for( player_t *play = all_players->next; play->id != 0; play = play->next )
        send(play->socket, msg, strlen(msg), 0);
}
