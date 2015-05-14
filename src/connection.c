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
#include <unistd.h>
#include <string.h>
#include "connection.h"

void *connect_client(void *sock){
    const int size = 2048;
    int id;
    int read_len;
    int l_sock = *(int *)sock;
    char msg_in[size];
    char msg_out[size];

    free(sock);

    pthread_mutex_lock(&lock);

    id = connect_c + 1;
    connect_c++;

    pthread_mutex_unlock(&lock);

    sprintf(msg_out, "Hello, you are #%d!!\n", id);
    write(l_sock, msg_out, strlen(msg_out));

    sprintf(msg_out, "Connection Successfull!!\n");
    write(l_sock, msg_out, strlen(msg_out));

    /* devolve mensagem enviada */
    while( (read_len = recv(l_sock, msg_in, size, 0)) > 0 ){
        write(l_sock, msg_in, strlen(msg_in));
        memset(msg_in, 0, size);
    }

    return 0;
}
