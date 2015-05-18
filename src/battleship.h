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

#ifndef H_BATTLESHIP
#define H_BATTLESHIP

#include <pthread.h>
#include <stdbool.h>

#define MAX_PLAYERS 50

/*  global variables    */
static int *conn_sock[MAX_PLAYERS];
static char buffer[256];
static pthread_mutex_t game_msg_lock;
static pthread_cond_t game_msg_cond;

/*  data types  */
enum celula{
    hit,
    water,
    torpedo,
    carrier,
    submarine,
    battleship
};
typedef enum celula cell_t;

struct navio{
    int points;
    bool sink;
    cell_t *posicao[];
};
typedef struct navio navio_t;

/*  function declarations   */
void game_setup();

void *broadcast_game(void*);

#endif
