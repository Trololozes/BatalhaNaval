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

#define PORT 5824
#define PENDING 5

struct info{
    int sock;
    struct sockaddr_in addr;
    game_t *game;
};
typedef struct info info_t;

void *listener(void*);
void *connect_client(void*);

static volatile int connect_c = 0;
static pthread_mutex_t counter_lock;
static pthread_mutex_t timeout_lock;
static pthread_cond_t timeout_cond;
static pthread_barrier_t listen_bar;

bool run_Forrest_run = true;
pthread_mutex_t sock_kill_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sock_kill_cond = PTHREAD_COND_INITIALIZER;

player_t *all_players[MAX_PLAYERS] = { NULL };

int main(int argc, char *argv[]){
    struct sockaddr_in serv_addr;
    pthread_t close_thr;
    pthread_t listen_thr;
    info_t specs;

    pthread_mutex_init(&counter_lock, NULL);
    pthread_cond_init(&timeout_cond, NULL);
    pthread_mutex_init(&timeout_lock, NULL);

    pthread_cond_init(&sock_kill_cond, NULL);
    pthread_mutex_init(&sock_kill_lock, NULL);
    signal(SIGINT, sighandler);

    pthread_barrier_init(&listen_bar, NULL, 2);

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

    specs.game = game_setup();

    pthread_create(&listen_thr, NULL, listener, &specs);

    while( run_Forrest_run && connect_c < 2 ){
        sleep(15);

        if( connect_c )
            broadcast_game("Aguardando jogadores...\n");
    }

    broadcast_game("Shall we play a game?\n");
    broadcast_game("Turno: Player#1\n");

    pthread_barrier_wait(&listen_bar);

    pthread_cancel(close_thr);

    for( int i = 0; i < MAX_PLAYERS; i++ ){
        if( all_players[i] != NULL ){
            write(all_players[i]->socket, "Server down\n", 12);
            free(all_players[i]);
            all_players[i] = NULL;
        }
    }

    close(specs.sock);
    specs.game = game_cleanup(specs.game);

    pthread_mutex_destroy(&counter_lock);
    pthread_mutex_destroy(&sock_kill_lock);
    pthread_cond_destroy(&sock_kill_cond);
    pthread_mutex_destroy(&timeout_lock);
    pthread_cond_destroy(&timeout_cond);

    exit(EXIT_SUCCESS);
}

void *listener(void *info){
    socklen_t sock_len;
    player_t **player = all_players;
    info_t *specs = (info_t *)info;

    while( run_Forrest_run ){
        *player = malloc(sizeof **player);
        sock_len = sizeof ( struct sockaddr_in );

        (**player).game = specs->game;
        (**player).pontos = 0;

        (**player).socket = \
            accept(specs->sock, (struct sockaddr *)&specs->addr, &sock_len);
        if( ! run_Forrest_run ) break;
        if( (**player).socket < 0 ) continue;

        if( pthread_create(&(**player).thread, NULL, connect_client, *player) ){
            close((**player).socket);
            free(*player);
            continue;
        }

        player++;
    }

    pthread_barrier_wait(&listen_bar);
    return 0;
}

void *connect_client(void *player){
    const int size = 2048;
    int linha;
    int coluna;
    int read_len;
    char msg_in[size];
    char msg_out[size];
    player_t *me = (player_t *)player;

    pthread_mutex_lock(&counter_lock);
    me->id = connect_c + 1;
    connect_c++;
    pthread_mutex_unlock(&counter_lock);

    sprintf(msg_out, "Connection Successfull!!\n"
        "You are Player#%d\n", me->id);
    write(me->socket, msg_out, strlen(msg_out));
    memset(msg_out, 0, size);

    while( (read_len = recv(me->socket, msg_in, size, 0)) > 0 ){
        sscanf(msg_in, "%d*%d", &linha, &coluna);

        me->pontos += game_fire(linha, coluna, me->game);

        memset(msg_in, 0, size);
    }

    pthread_mutex_lock(&counter_lock);
    fprintf(stdout, "Connection #%d closed\n", me->id);
    connect_c--;
    pthread_mutex_unlock(&counter_lock);

    return 0;
}
