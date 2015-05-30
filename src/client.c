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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <ncurses.h>
#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "signal_handler.h"

/*
 *  Global constants
 */
#define PORT 5824
#define BUFF_S 256

/*
 *  External global variables
 */
bool run_Forrest_run = true;
pthread_mutex_t sock_kill_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sock_kill_cond = PTHREAD_COND_INITIALIZER;

/*
 *  Data types
 */
struct out{
    WINDOW *win;
    PANEL *panel;
    int socket;
};
typedef struct out out_t;

/*
 *  Global variables within this file
 */
static pthread_mutex_t send_lock;
static pthread_cond_t send_cond;

/*
 *  Functions declarations
 */
void *outgoing_msgs(void*);

/*
 *  Main definition
 */
int main(int argc, char *argv[]){
    int sock;
    int id = 0;
    int turn = 0;
    char buff[BUFF_S];
    char *ip;
    struct sockaddr_in serv_addr;
    pthread_t close_thr;
    pthread_t input_thr;
    WINDOW *outgoing;
    WINDOW *incoming;
    PANEL *out_panel;
    out_t info;

    if( argc != 2 ){
        fprintf(stderr, "Usage: %s [HOST]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ip = argv[1];

    pthread_mutex_init(&send_lock, NULL);
    pthread_cond_init(&send_cond, NULL);

    pthread_mutex_init(&sock_kill_lock, NULL);
    pthread_cond_init(&sock_kill_cond, NULL);
    signal(SIGINT, sighandler);

    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        fprintf(stderr, "socket: %s\n", strerror(errno));
    }

    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0 ){
        fprintf(stderr, "connect(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    initscr();
    nonl();
    cbreak();

    outgoing = newwin(5, COLS , LINES-5, 0);
    out_panel = new_panel(outgoing);
    incoming = newwin(LINES-5, COLS, 0, 0);
    scrollok(incoming, TRUE);

    info.win = outgoing;
    info.panel = out_panel;
    info.socket = sock;

    pthread_create(&close_thr, NULL, close_socket, &sock);
    pthread_create(&input_thr, NULL, outgoing_msgs, &info);

    memset(buff, 0, BUFF_S);
    while( recv(sock, buff, BUFF_S, 0) > 0 ){
        waddstr(incoming, buff);

        sscanf(buff, "%*[^#]#%d", ( ! id ) ? &id : &turn);
        memset(buff, 0, BUFF_S);

        if( turn == id ){
            pthread_mutex_lock(&send_lock);
            pthread_cond_broadcast(&send_cond);
            pthread_mutex_unlock(&send_lock);
        }

        wrefresh(incoming);
    }

    endwin();

    close(sock);
    pthread_cancel(input_thr);
    pthread_cancel(close_thr);

    pthread_mutex_destroy(&send_lock);
    pthread_cond_destroy(&send_cond);
    pthread_mutex_destroy(&sock_kill_lock);
    pthread_cond_destroy(&sock_kill_cond);

    exit(EXIT_SUCCESS);
}

/*
 *  Functions definitions
 */
void *outgoing_msgs(void *out){
    int xy[2];
    char buff[BUFF_S];
    char tmp[10];
    char *lin_col[2];
    out_t info = *(out_t *)out;

    lin_col[0] = "Digite a linha [0-99]: ";
    lin_col[1] = "Digite a coluna [0-99]: ";

    while( run_Forrest_run ){
        pthread_mutex_lock(&send_lock);
        pthread_cond_wait(&send_cond, &send_lock);

        box(info.win, 0, 0);
        show_panel(info.panel);

        for( int i = 0; i < 2; i++ ){
            mvwaddstr(info.win, 2, 1, lin_col[i]);
            wgetnstr(info.win, tmp, 10);
            sscanf(tmp, "%d\n", &xy[i]);
            memset(tmp, 0, 10);
        }

        wclear(info.win);

        update_panels();
        doupdate();

        sprintf(buff, "%d*%d", xy[0], xy[1]);
        send(info.socket, buff, strlen(buff), 0);
        memset(buff, 0, BUFF_S);

        hide_panel(info.panel);

        pthread_mutex_unlock(&send_lock);
    }

    return 0;
}
