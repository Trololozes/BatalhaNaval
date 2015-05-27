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
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "battleship.h"

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
    struct timeval precision;
    navio_t *boat;

    gettimeofday(&precision, NULL);
    srand((precision.tv_sec) + (precision.tv_usec));
    rand();

    switch(ship){
        case torpedo:
            points = 15;
            width = TOR_W;
            size = TOR_N;
            boat = game->torpedeiro;
            break;
        case carrier:
            points = 20;
            width = PAV_W;
            size = PAV_N;
            boat = game->porta_aviao;
            break;
        case submarine:
            points = 35;
            width = SUB_W;
            size = SUB_N;
            boat = game->submarino;
            break;
        case battleship:
            points = 50;
            width = COU_W;
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
    cell_t **pos = calloc(width, sizeof **pos);

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

void game_fire(int x, int y, player_t *player){
    const int buff_s = 256;
    int size;
    int width;
    int points;
    char buffer[buff_s];
    char n_round[buff_s];
    bool check_ships = true;
    cell_t target;
    navio_t *ship;
    player_t *next;

    memset(buffer, 0, buff_s);
    memset(n_round, 0, buff_s);

    points = 0;

    target = player->game->grid[x][y];
    player->game->grid[x][y] = hit;

    sprintf(buffer, "[%d, %d] -> ", x, y);

    switch(target){
        case torpedo:
            strncat(buffer, "Torpedeiro!", buff_s);
            size = TOR_N;
            width = TOR_W;
            ship = player->game->torpedeiro;
            break;
        case carrier:
            strncat(buffer, "Porta-Avioes!", buff_s);
            size = PAV_N;
            width = PAV_W;
            ship = player->game->porta_aviao;
            break;
        case submarine:
            strncat(buffer, "Submarino!", buff_s);;
            size = SUB_N;
            width = SUB_W;
            ship = player->game->submarino;
            break;
        case battleship:
            strncat(buffer, "Couracado!", buff_s);
            size = COU_N;
            width = COU_W;
            ship = player->game->couracado;
            break;
        case water:
            strncat(buffer, "Splash!", buff_s);
            check_ships = false;
            break;
        case hit:
            strncat(buffer, "Nope, aqui ja foi atacado antes", buff_s);
            check_ships = false;
            break;
    }

    next = ( player->next->id == 0 ) ? player->next->next : player->next;

    if( check_ships ){
        next = player;
        points = is_sink(ship, size, width);
        strncat(buffer, ( points ) ? " - Afundado!" : " - Atingido!", buff_s);
        player->pontos += points;
    }

    strncat(buffer, "\n", buff_s);
    broadcast_game(buffer);

    sprintf(n_round, "Nova rodada - (Pontuacao: %d)Player#%d\n", \
            next->pontos, next->id);
    broadcast_game(n_round);

    return;
}

int is_sink(navio_t *ship, int size, int width){
    int c;
    int points = 0;
    cell_t *pos = NULL;
    navio_t *head = NULL;

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

void broadcast_game(char *msg){
    for( player_t *play = all_players->next; play->id != 0; play = play->next )
        write(play->socket, msg, strlen(msg));
}
