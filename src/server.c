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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "signal_handler.h"
#include "battleship.h"
#include "stack.h"

/*
 *  Global constants
 */
#define PORT 5824
#define PENDING 5

/*
 *  External global variables
 */
bool run_Forrest_run = true;
pthread_mutex_t sock_kill_lock;
pthread_cond_t sock_kill_cond;
pthread_barrier_t end_game_bar;
player_t *all_players = NULL;
game_t *game_ptr = NULL;

/*
 *  Data types
 */
struct local_info{
    int sock;
    struct sockaddr_in addr;
};
typedef struct local_info local_info_t;

/*
 *  Global variables within this file
 */
static volatile int connect_c = 0;
static pthread_mutex_t counter_lock;
static pthread_mutex_t timeout_lock;
static pthread_cond_t timeout_cond;

/*
 *  Functions definitions
 */
void *listener(void*);
void *connect_client(void*);

/*
 *  Main definition
 */
int main(int argc, char *argv[]){
    char first_player[BUFF_S];
    struct sockaddr_in serv_addr;
    pthread_t close_thr;
    pthread_t listen_thr;
    local_info_t specs;

    memset(first_player, 0, BUFF_S);

    pthread_mutex_init(&counter_lock, NULL);
    pthread_cond_init(&timeout_cond, NULL);
    pthread_mutex_init(&timeout_lock, NULL);

    pthread_cond_init(&sock_kill_cond, NULL);
    pthread_mutex_init(&sock_kill_lock, NULL);
    signal(SIGINT, sighandler);

    pthread_barrier_init(&end_game_bar, NULL, 2);

    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if( (specs.sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() error\n");
        perror(strerror(errno));
    }

    if( bind(specs.sock, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0 ){
        perror("bind() error\n");
        perror(strerror(errno));
    }

    listen(specs.sock, PENDING);
    pthread_create(&close_thr, NULL, close_socket, &specs.sock);

    stack_up(MAX_PLAYERS);
    game_setup();

    all_players = malloc(sizeof *all_players);
    all_players->id = 0;
    all_players->next = all_players;
    all_players->prev = all_players;

    pthread_create(&listen_thr, NULL, listener, &specs);

    while( run_Forrest_run && connect_c < 2 ){
        sleep(15);

        if( connect_c )
            game_broadcast("Aguardando jogadores...\n");
    }

    game_broadcast("Shall we play a game?\n");
    sprintf(first_player, "Turno: Player#%d\n", all_players->next->id);
    game_broadcast(first_player);

    pthread_barrier_wait(&end_game_bar);

    pthread_cancel(close_thr);

    game_broadcast("Server desligando... Fim de jogo!\n");

    close(specs.sock);

    stack_destroy();
    game_cleanup();

    pthread_mutex_destroy(&counter_lock);
    pthread_mutex_destroy(&sock_kill_lock);
    pthread_cond_destroy(&sock_kill_cond);
    pthread_mutex_destroy(&timeout_lock);
    pthread_cond_destroy(&timeout_cond);

    exit(EXIT_SUCCESS);
}

/*
 *  Functions definitions
 */
void *listener(void *info){
    int tmp;
    char *sorry;
    socklen_t length;
    player_t *player;
    local_info_t *specs = (local_info_t *)info;

    length = sizeof ( struct sockaddr_in );

    while( run_Forrest_run ){
        tmp = accept(specs->sock, (struct sockaddr *)&specs->addr, &length);
        if( ! run_Forrest_run ) break;
        if( tmp < 0 ) continue;

        if( stack_empty() ){
            sorry = "Numero maximo de conexoes atingido, sorry\n";
            write(tmp, sorry, strlen(sorry));

            close(tmp);

            continue;
        }

        player = malloc(sizeof *player);

        player->tiros = MAX_PLAYERS_SHOTS;
        player->pontos = 0;
        player->socket = tmp;

        player->prev = all_players->prev;
        player->next = all_players;
        all_players->prev->next = player;
        all_players->prev = player;

        pthread_mutex_lock(&counter_lock);
        player->id = stack_pop();

        if( pthread_create(&player->thread, NULL, connect_client, player) ){
            close(player->socket);
            free(player);

            pthread_mutex_unlock(&counter_lock);

            continue;
        }

        connect_c++;
        pthread_mutex_unlock(&counter_lock);

        player++;
    }

    pthread_barrier_wait(&end_game_bar);

    return 0;
}

void *connect_client(void *player){
    int linha;
    int coluna;
    int read_len;
    char msg[BUFF_S];
    player_t *me = (player_t *)player;

    sprintf(msg, "Connection Successfull!!\n"
        "You are Player#%d\n", me->id);
    write(me->socket, msg, strlen(msg));
    memset(msg, 0, BUFF_S);

    while( (read_len = recv(me->socket, msg, BUFF_S, 0)) > 0 ){
        sscanf(msg, "%d*%d", &linha, &coluna);

        game_fire(linha, coluna, me);

        memset(msg, 0, BUFF_S);
    }

    pthread_mutex_lock(&counter_lock);
    fprintf(stdout, "Connection #%d closed\n", me->id);

    me->next->prev = me->prev;
    me->prev->next = me->next;
    stack_push(me->id);
    free(me);

    connect_c--;
    pthread_mutex_unlock(&counter_lock);

    return 0;
}
