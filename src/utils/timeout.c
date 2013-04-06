/*
    Copyright (c) 2012 250bpm s.r.o.

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

#include "timeout.h"
#include "fast.h"
#include "cont.h"
#include "err.h"

void nn_timeout_init (struct nn_timeout *self)
{
    nn_clock_init (&self->clock);
    nn_list_init (&self->timeouts);
}

void nn_timeout_term (struct nn_timeout *self)
{
    nn_list_term (&self->timeouts);
    nn_clock_term (&self->clock);
}

void nn_timeout_add (struct nn_timeout *self, int timeout,
    struct nn_timeout_hndl *hndl)
{
    struct nn_list_item *it;
    struct nn_timeout_hndl *ith;

    /*  Compute the instant when the timeout will be due. */
    hndl->timeout = nn_clock_now (&self->clock) + timeout;

    /*  Insert it into the ordered list of timeouts. */
    for (it = nn_list_begin (&self->timeouts);
          it != nn_list_end (&self->timeouts);
          it = nn_list_next (&self->timeouts, it)) {
        ith = nn_cont (it, struct nn_timeout_hndl, list);
        if (hndl->timeout < ith->timeout)
            break;
    }
    nn_list_insert (&self->timeouts, &hndl->list, it);
}

void nn_timeout_rm (struct nn_timeout *self, struct nn_timeout_hndl *hndl)
{
    nn_list_erase (&self->timeouts, &hndl->list);
}

int nn_timeout_timeout (struct nn_timeout *self)
{
    int timeout;

    if (nn_fast (nn_list_empty (&self->timeouts)))
        return -1;

    timeout = (int) (nn_cont (nn_list_begin (&self->timeouts),
        struct nn_timeout_hndl, list)->timeout - nn_clock_now (&self->clock));
    return timeout < 0 ? 0 : timeout;
}

int nn_timeout_event (struct nn_timeout *self, struct nn_timeout_hndl **hndl)
{
    struct nn_timeout_hndl *first;

    /*  If there's no timeout, there's no event to report. */
    if (nn_fast (nn_list_empty (&self->timeouts)))
        return -EAGAIN;

    /*  If no timeout have expired yet, there's no event to return. */
    first = nn_cont (nn_list_begin (&self->timeouts),
        struct nn_timeout_hndl, list);
    if (first->timeout > nn_clock_now (&self->clock))
        return -EAGAIN;

    /*  Return the first timeout and remove it from the list of active
        timeouts. */
    nn_list_erase (&self->timeouts, &first->list);
    *hndl = first;
    return 0;
}

void nn_timeout_hndl_init (struct nn_timeout_hndl *self)
{
    nn_list_item_init (&self->list);
}

void nn_timeout_hndl_term (struct nn_timeout_hndl *self)
{
    nn_list_item_term (&self->list);
}

int nn_timeout_hndl_isactive (struct nn_timeout_hndl *self)
{
    return nn_list_item_isinlist (&self->list);
}

