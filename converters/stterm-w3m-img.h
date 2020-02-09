/*
 * Copyright (c) 2014 haru <uobikiemukot at gmail dot com>
 * Copyright (c) 2020 Joseph Benden <joe@benden.us>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
*/

#ifndef LIBSIXEL_CONVERTERS_STTERM_W3M_IMG_H
#define LIBSIXEL_CONVERTERS_STTERM_W3M_IMG_H

#ifdef __cplusplus
extern "C" {
#endif

enum w3m_op {
    W3M_DRAW = 0,
    W3M_REDRAW,
    W3M_STOP,
    W3M_SYNC,
    W3M_NOP,
    W3M_GETSIZE,
    W3M_CLEAR,
    NUM_OF_W3M_FUNC,
};

enum {
    VERBOSE            = true,   /* if false, suppress "DEBUG" level logging */
    BUFSIZE            = 1024,
    MAX_IMAGE          = 1024,
    /* default value */
    CELL_WIDTH         = 8,
    CELL_HEIGHT        = 16,
    /* this values will be updated at window resize */
    TERM_WIDTH         = 1280,
    TERM_HEIGHT        = 1024,
    /* for select */
    SELECT_TIMEOUT     = 100000, /* usec */
    SELECT_CHECK_LIMIT = 4,
};

struct tty_t {
    int fd;                      /** fd of current controlling terminal */
    int width, height;           /** terminal size (in pixels) */
    int cell_width, cell_height; /** cell_size (in pixels) */
};

static const char *log_file     = "/tmp/w3mimg-sixel.log";

#if HAVE_SIGNAL
volatile sig_atomic_t window_resized = false; /** set on SIGWINCH */
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIBSIXEL_CONVERTERS_STTERM_W3M_IMG_H */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
