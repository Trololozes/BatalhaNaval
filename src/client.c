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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define PORT 5824

int main(int argc, char *argv[]){
    const int size = 256;
    int sock;
    int read_len;
    char buff[size];
    char *ip;
    struct sockaddr_in serv_addr;

    if( argc != 2 ){
        perror("Error");
        exit(EXIT_FAILURE);
    }

    ip = argv[1];

    if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() error\n");
        perror(strerror(errno));
    }

    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0 ){
        perror("connect() error\n");
        perror(strerror(errno));
    }

    while( (read_len = recv(sock, buff, size, 0)) > 0 ){
        puts(buff);

        memset(buff, 0, size);
        fgets(buff, size, stdin);

        send(sock, buff, strlen(buff), 0);
        memset(buff, 0, size);
    }

    exit(EXIT_SUCCESS);
}
