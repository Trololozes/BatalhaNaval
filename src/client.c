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
#include <regex.h>
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
 *  Functions declarations
 */
void send_xy(out_t*);

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
    WINDOW *outgoing;
    WINDOW *in_stats;
    WINDOW *in_turn;
    PANEL *out_panel;
    regex_t re_stat;
    regex_t re_turn;
    regmatch_t m_stat;
    regmatch_t m_turn;
    out_t info;

    if( argc != 2 ){
        fprintf(stderr, "Usage: %s [HOST]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ip = argv[1];

    pthread_mutex_init(&sock_kill_lock, NULL);
    pthread_cond_init(&sock_kill_cond, NULL);
    signal(SIGINT, sighandler);

    regcomp(&re_stat, "^[><-]{2}[^=]*\n", REG_EXTENDED);
    regcomp(&re_turn, "==[^><-]*\n", REG_EXTENDED);

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
    start_color();
    nonl();
    cbreak();

    init_pair(1, COLOR_BLACK, COLOR_RED);

    outgoing = newwin(3, COLS , LINES-3, 0);
    out_panel = new_panel(outgoing);
    in_turn = newwin(1, COLS, LINES-4, 0);
    in_stats = newwin(LINES-4, COLS, 0, 0);
    scrollok(in_stats, TRUE);

    info.win = outgoing;
    info.panel = out_panel;
    info.socket = sock;

    hide_panel(out_panel);
    update_panels();

    pthread_create(&close_thr, NULL, close_socket, &sock);

    memset(buff, 0, BUFF_S);
    while( recv(sock, buff, BUFF_S, 0) > 0 ){
        if( ! regexec(&re_stat, buff, 1, &m_stat, 0) )
            waddnstr(in_stats, buff+m_stat.rm_so, m_stat.rm_eo-m_stat.rm_so);
        if( ! regexec(&re_turn, buff, 1, &m_turn, 0) ){
            wclear(in_turn);
            waddnstr(in_turn, buff+m_turn.rm_so, m_turn.rm_eo-m_turn.rm_so);
        }

        sscanf(buff, "%*[^#]#%d", ( ! id ) ? &id : &turn);
        memset(buff, 0, BUFF_S);

        wnoutrefresh(in_turn);
        wnoutrefresh(in_stats);

        if( turn == id )
            send_xy(&info);

        doupdate();
    }

    close(sock);
    pthread_cancel(close_thr);

    regfree(&re_stat);
    regfree(&re_turn);

    pthread_mutex_destroy(&sock_kill_lock);
    pthread_cond_destroy(&sock_kill_cond);

    box(outgoing, 0, 0);
    mvwaddstr(outgoing, 1, 1, "Aperte qualquer tecla para sair...");
    wgetch(outgoing);
    update_panels();
    doupdate();
    endwin();

    exit(EXIT_SUCCESS);
}

/*
 *  Functions definitions
 */
void send_xy(out_t *info){
    int xy[2];
    char buff[BUFF_S];
    char tmp[5];
    char *lin_col[2];
    char *end;
    int (*attr_ch)(WINDOW*, int);

    lin_col[0] = "Digite a linha [0-99]: ";
    lin_col[1] = "Digite a coluna [0-99]: ";

    flushinp();
    show_panel(info->panel);

    for( int i = 0; i < 2; i++ ){
        attr_ch = wattroff;

        do{
            attr_ch(info->win, COLOR_PAIR(1));

            memset(tmp, 0, 5);

            wclear(info->win);
            box(info->win, 0, 0);

            mvwaddstr(info->win, 1, 1, lin_col[i]);
            wgetnstr(info->win, tmp, 5);
            xy[i] = strtol(tmp, &end, 10);

            attr_ch = wattron;
        }while( tmp == end || *end != 0 || xy[i] < 0 || xy[i] > 99 );
    }

    sprintf(buff, "%d*%d", xy[0], xy[1]);
    send(info->socket, buff, strlen(buff), 0);
    memset(buff, 0, BUFF_S);

    hide_panel(info->panel);
    update_panels();

    return;
}
