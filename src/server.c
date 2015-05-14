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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "connection.h"

#define PORT 5824
#define PENDING 5

int main(int argc, char *argv[]){
    int list_sock;
    int *conn_sock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t sock_len;
    pthread_t thread;

    if( (list_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() error\n");
        perror(strerror(errno));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if( bind(list_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){
        perror("bind() error\n");
        perror(strerror(errno));
    }

    listen(list_sock, PENDING);

    pthread_mutex_init(&lock, NULL);

    while( true ){
        conn_sock = malloc(sizeof *conn_sock);
        sock_len = sizeof(struct sockaddr_in);

        *conn_sock = accept(list_sock, (struct sockaddr *)&cli_addr, &sock_len);
        if( *conn_sock < 0 ){
            perror("accept() error\n");
            perror(strerror(errno));
        }

        if( pthread_create(&thread, NULL, connect_client, (void *)conn_sock) ){
            close(list_sock);
            close(*conn_sock);
        }
    }

    pthread_mutex_destroy(&lock);

    exit(EXIT_SUCCESS);
}
