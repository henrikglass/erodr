
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_chan.h implements a thread-safe message queue similar to go channels.
 *
 *
 * USAGE:
 *
 * Include hgl_chan.h file like this:
 *
 *     #define HGL_CHAN_IMPLEMENTATION
 *     #include "hgl_chan.h"
 * 
 *
 * EXAMPLE:
 * 
 * See the examples directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_CHAN_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdbool.h>
#include <pthread.h>
#include <stdarg.h>

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    void *item;
    pthread_mutex_t mutex;
    pthread_cond_t cvar_writable;
    pthread_cond_t cvar_readable;
    bool readable;
    int efd;
} HglChan;

/**
 * Creates a channel.
 */
HglChan hgl_chan_make(void);

/**
 * Destroys the channel `c`.
 */
void hgl_chan_destroy(HglChan *c);

/**
 * Sends item `item` on the channel `c`. Will block if there already 
 * is an item in `c`.
 */
void hgl_chan_send(HglChan *c, void *item);

/**
 * Tries to send the item `item` on the channel `c`. If there already is 
 * an item on `c`, `item` will not be sent and -1 will be returned, otherwise,
 * if `item` was successfully sent, 0 will be returned.
 */
int hgl_chan_try_send(HglChan *c, void *item);

/**
 * Recieve an item from the channel `c`. Will block until there is an item
 * on `c`.
 */
void *hgl_chan_recv(HglChan *c);

/**
 * Recieve an item from the channel `c`. Will return immediately return NULL 
 * if there is no item on `c`. If there is an item on `c`, it will be returned.
 */
void *hgl_chan_try_recv(HglChan *c);

/**
 * Wait on any number (<= 128) of channels simultaneously, until at least one
 * of them becomes readable (i.e. something was sent on the channel). Returns
 * a pointer to the first readable channel in the variadic arguments list.
 */
HglChan *hgl_chan_select(int n_args, ...);

/**
 * Returns a pointer to the first readable channel in the variadic arguments 
 * list. If there are no readable channels NULL is returned.
 */
HglChan *hgl_chan_try_select(int n_args, ...);

#endif

#ifdef HGL_CHAN_IMPLEMENTATION

#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>

HglChan hgl_chan_make()
{
    int err;
    HglChan c;
    c.readable = false;
    err  = pthread_mutex_init(&c.mutex, NULL);
    err |= pthread_cond_init(&c.cvar_writable, NULL);
    err |= pthread_cond_init(&c.cvar_readable, NULL);
    c.efd = eventfd(0, 0);
    if (err != 0) {
        fprintf(stderr, "[hgl_chan.h]: Failed to create channel.\n");
    }
    return c;
}

void hgl_chan_destroy(HglChan *c)
{
    pthread_mutex_destroy(&c->mutex);
    pthread_cond_destroy(&c->cvar_writable);
    pthread_cond_destroy(&c->cvar_readable);
    close(c->efd);
}

void hgl_chan_send(HglChan *c, void *item)
{
    pthread_mutex_lock(&c->mutex);
    while (c->readable /* implies not writable */) {
        pthread_cond_wait(&c->cvar_writable, &c->mutex);
    }
    c->item = item;
    c->readable = true;
    uint64_t efd_inc = 1;
    ssize_t n_bytes_written = write(c->efd, &efd_inc, sizeof(uint64_t)); // increment counter
    if (n_bytes_written != sizeof(uint64_t)) {
        fprintf(stderr, "[hgl_chan.h]: Aborted <%s, %d>\n", __FILE__, __LINE__);
        abort(); // Temporary, TODO handle this properly
    }
    pthread_cond_signal(&c->cvar_readable);
    pthread_mutex_unlock(&c->mutex);
}

int hgl_chan_try_send(HglChan *c, void *item)
{
    pthread_mutex_lock(&c->mutex);
    if (c->readable /* impl. not writable */) {
        pthread_mutex_unlock(&c->mutex);
        return -1;
    }
    c->item = item;
    c->readable = true;
    uint64_t efd_inc = 1;
    ssize_t n_bytes_written = write(c->efd, &efd_inc, sizeof(uint64_t)); // increment counter
    if (n_bytes_written != sizeof(uint64_t)) {
        fprintf(stderr, "[hgl_chan.h]: Aborted <%s, %d>\n", __FILE__, __LINE__);
        abort(); // Temporary, TODO handle this properly
    }
    pthread_cond_signal(&c->cvar_readable);
    pthread_mutex_unlock(&c->mutex);
    return 0;
}

void *hgl_chan_recv(HglChan *c)
{
    pthread_mutex_lock(&c->mutex);
    while (!c->readable) {
        pthread_cond_wait(&c->cvar_readable, &c->mutex);
    }
    void *item = c->item;
    c->readable = false;
    uint64_t efd_counter;
    read(c->efd, &efd_counter, sizeof(uint64_t)); // resets counter to zero
    pthread_cond_signal(&c->cvar_writable);
    pthread_mutex_unlock(&c->mutex);
    return item;
}

void *hgl_chan_try_recv(HglChan *c)
{
    pthread_mutex_lock(&c->mutex);
    if (!c->readable) {
        pthread_mutex_unlock(&c->mutex);
        return NULL;
    }
    void *item = c->item;
    c->readable = false;
    uint64_t efd_counter;
    read(c->efd, &efd_counter, sizeof(uint64_t)); // resets counter to zero
    pthread_cond_signal(&c->cvar_writable);
    pthread_mutex_unlock(&c->mutex);
    return item;
}

HglChan *hgl_chan_select(int n_args, ...)
{
    HglChan *ret = NULL;
    static struct pollfd pfds[128]; // 128 should be more than enough

    if (n_args > 128) {
        fprintf(stderr, "[hgl_chan.h] Error: too many (> 128) channels passed to `hgl_chan_select`.\n");
        return NULL;
    }

    /* setup va_list */
    va_list args1, args2;    
    va_start(args1, n_args);
    va_copy(args2, args1);

    /* populate pfds */
    for (int i = 0; i < n_args; i++) {
        HglChan *c = va_arg(args1, HglChan *);
        pfds[i].fd = c->efd;
        pfds[i].events = POLLIN;
    }

    /* poll: wait (forever) till one of the channels becomes readable */
    poll(pfds, (nfds_t) n_args, -1);

    /* find the first readable channel */
    for (int i = 0; i < n_args; i++) {
        HglChan *c = va_arg(args2, HglChan *);
        if (c->readable) {
            ret = c;
            break;
        }
    }

    va_end(args1);
    va_end(args2);
    return ret;
}

HglChan *hgl_chan_try_select(int n_args, ...)
{
    HglChan *ret = NULL;

    /* setup va_list */
    va_list args;    
    va_start(args, n_args);

    for (int i = 0; i < n_args; i++) {
        HglChan *c = va_arg(args, HglChan *);
        if (c->readable) {
            ret = c;
            break;
        }  
    }

    va_end(args);
    return ret;
}

#endif

