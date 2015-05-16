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

#define PORT 5824
#define PENDING 5
#define MAX_CONN 50

void sighandler(int);
void *stop_listening(void*);
void *connect_client(void*);

static volatile int connect_c = 0;
static bool run_Forrest_run = true;
static pthread_mutex_t counter_lock;
static pthread_mutex_t sock_kill_lock;
static pthread_cond_t sock_kill_cond;

int main(int argc, char *argv[]){
    int list_sock;
    int *conn_sock[MAX_CONN] = { NULL };
    int **conn_ptr;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    socklen_t sock_len;
    pthread_t thread;

    pthread_cond_init(&sock_kill_cond, NULL);
    pthread_mutex_init(&sock_kill_lock, NULL);
    pthread_mutex_init(&counter_lock, NULL);

    conn_ptr = conn_sock;

    signal(SIGINT, sighandler);

    if( (list_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() error\n");
        perror(strerror(errno));
    }

    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if( bind(list_sock, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0 ){
        perror("bind() error\n");
        perror(strerror(errno));
    }

    listen(list_sock, PENDING);
    pthread_create(&thread, NULL, stop_listening, (void *)&list_sock);

    while( run_Forrest_run ){
        *conn_ptr = malloc(sizeof **conn_ptr);
        sock_len = sizeof ( struct sockaddr_in );

        **conn_ptr = accept(list_sock, (struct sockaddr *)&cli_addr, &sock_len);
        if( **conn_ptr < 0 ){
            perror("accept() error\n");
            perror(strerror(errno));
            continue;
        }

        if( pthread_create(&thread, NULL, connect_client, (void *)*conn_ptr) ){
            close(**conn_ptr);
            free(*conn_ptr);
            conn_ptr--;
        }

        conn_ptr++;
    }

    for( int i = 0; i < MAX_CONN; i++ ){
        if( conn_sock[i] != NULL ){
            write(*conn_sock[i], "Server down\n", 12);
            free(conn_sock[i]);
            conn_sock[i] = NULL;
        }
    }

    pthread_mutex_destroy(&counter_lock);
    pthread_mutex_destroy(&sock_kill_lock);
    pthread_cond_destroy(&sock_kill_cond);

    exit(EXIT_SUCCESS);
}

void sighandler(int sig){
    pthread_mutex_lock(&sock_kill_lock);
    pthread_cond_broadcast(&sock_kill_cond);

    run_Forrest_run = false;

    pthread_mutex_unlock(&sock_kill_lock);
}

void *stop_listening(void *sock){
    pthread_mutex_lock(&sock_kill_lock);
    pthread_cond_wait(&sock_kill_cond, &sock_kill_lock);

    shutdown(*(int *)sock, SHUT_RD);

    return 0;
}

void *connect_client(void *sock){
    const int size = 2048;
    int id;
    int read_len;
    int l_sock = *(int *)sock;
    char msg_in[size];
    char msg_out[size];

    pthread_mutex_lock(&counter_lock);
    id = connect_c + 1;
    connect_c++;
    pthread_mutex_unlock(&counter_lock);

    sprintf(msg_out, "Hello, you are #%d!!\n", id);
    write(l_sock, msg_out, strlen(msg_out));

    sprintf(msg_out, "Connection Successfull!!\n");
    write(l_sock, msg_out, strlen(msg_out));

    /* devolve mensagem enviada */
    while( (read_len = recv(l_sock, msg_in, size, 0)) > 0 ){
        write(l_sock, msg_in, strlen(msg_in));
        memset(msg_in, 0, size);
    }

    pthread_mutex_lock(&counter_lock);
    fprintf(stdout, "Connection #%d closed\n", id);
    connect_c--;
    pthread_mutex_unlock(&counter_lock);

    return 0;
}
