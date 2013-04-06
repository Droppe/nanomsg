/*
    Copyright (c) 2013 250bpm s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef NN_WORKER_INCLUDED
#define NN_WORKER_INCLUDED

#include "queue.h"
#include "mutex.h"
#include "thread.h"
#include "efd.h"
#include "poller.h"
#include "timeout.h"
#include "async.h"

struct nn_worker_fd {
    struct nn_async *owner;
    struct nn_poller_hndl phndl;
};

void nn_worker_fd_init (struct nn_worker_fd *self, struct nn_async *owner);
void nn_worker_fd_term (struct nn_worker_fd *self);

struct nn_worker_task {
    struct nn_async *owner;
    struct nn_queue_item item;
};

void nn_worker_task_init (struct nn_worker_task *self, struct nn_async *owner);
void nn_worker_task_term (struct nn_worker_task *self);

struct nn_worker_timer {
    struct nn_async *owner;
    struct nn_timeout_hndl hndl;
};

void nn_worker_timer_init (struct nn_worker_timer *self,
    struct nn_async *owner);
void nn_worker_timer_term (struct nn_worker_timer *self);

struct nn_worker {
    struct nn_mutex sync;
    struct nn_queue events;
    struct nn_queue_item stop;
    struct nn_efd efd;
    struct nn_poller poller;
    struct nn_poller_hndl efd_hndl;
    struct nn_timeout timeout;
    struct nn_thread thread;
};

void nn_worker_init (struct nn_worker *self);
void nn_worker_term (struct nn_worker *self);

void nn_worker_post (struct nn_worker *self, struct nn_worker_task *task);

/*  Following functions should be used only from within the event handlers! */

void nn_worker_add (struct nn_worker *self, int s, struct nn_worker_fd *fd);
void nn_worker_rm (struct nn_worker *self, struct nn_worker_fd *fd);
void nn_worker_set_in (struct nn_worker *self, struct nn_worker_fd *fd);
void nn_worker_reset_in (struct nn_worker *self, struct nn_worker_fd *fd);
void nn_worker_set_out (struct nn_worker *self, struct nn_worker_fd *fd);
void nn_worker_reset_out (struct nn_worker *self, struct nn_worker_fd *fd);

int nn_worker_add_timer (struct nn_worker *self,  int timeout,
    struct nn_worker_timer *timer);
int nn_worker_rm_timer (struct nn_worker *self, struct nn_worker_timer *timer);

#endif

