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

#define _DEFAULT_SOURCE

#include "config.h"
#include "malloc_stub.h"

#if STDC_HEADERS
# include <ctype.h>
# include <limits.h>
# include <stdlib.h>
# include <stdarg.h>
# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>
#endif  /* STDC_HEADERS */
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif  /* HAVE_ERRNO_H */
#if HAVE_FCNTL_H
# include <fcntl.h>
#endif  /* HAVE_FCNTL_H */
#if HAVE_SIGNAL_H
# include <signal.h>
#endif
#if HAVE_SYS_SIGNAL_H
# include <sys/signal.h>
#endif
#if HAVE_STRING_H
# include <string.h>
#endif  /* HAVE_STRING_H */
#if HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif  /* HAVE_SYS_IOCTL_H */
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif  /* HAVE_SYS_SELECT_H */
#if HAVE_TERMIOS_H
# include <termios.h>
#endif  /* HAVE_TERMIOS_H */
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_UNISTD_H
# include <sys/unistd.h>
#endif

#include "stterm-w3m-img.h"
#include "stterm-util.h"
#include "parsearg.h"
#include "sixel.h"

#include "../src/encoder.h"

typedef struct sixel_callback_context_for_getsize {
    int width;
    int height;
} sixel_callback_context_for_getsize_t;


static void
w3m_draw(
    struct tty_t const * const  /* in */ tty,
    struct parm_t const * const /* in */ parm,
    int                         /* in */ op)
{
    SIXELSTATUS status = SIXEL_FALSE;
    sixel_encoder_t *encoder = NULL;
    int index, offset_x, offset_y, width, height;
    int shift_x, shift_y;
    int view_w, view_h;
    char *file;
    char buf[BUFSIZE];

    logging(DEBUG, "w3m_draw(%d)\n", op);

    if (parm->argc != 11)
        return;

    index     = str2num(parm->argv[1]) - 1; /* 1 origin */
    offset_x  = str2num(parm->argv[2]);
    offset_y  = str2num(parm->argv[3]);
    width     = str2num(parm->argv[4]);
    height    = str2num(parm->argv[5]);
    shift_x   = str2num(parm->argv[6]);
    shift_y   = str2num(parm->argv[7]);
    view_w    = str2num(parm->argv[8]);
    view_h    = str2num(parm->argv[9]);
    file      = parm->argv[10];

    if (index < 0)
        index = 0;
    else if (index >= MAX_IMAGE)
        index = MAX_IMAGE - 1;

    /* cursor move */
    snprintf(buf,
             BUFSIZE,
             "\033[%d;%dH",
             (offset_y / tty->cell_height) + 1,
             (offset_x / tty->cell_width) + 2);
    ewrite(tty->fd, buf, strlen(buf));

    logging(DEBUG, "x=%d y=%d cw=%d ch=%d\n",
            offset_x, offset_y,
            tty->cell_width, tty->cell_height);

    /* generate sixels */
    status = sixel_encoder_new(&encoder, NULL);
    if (SIXEL_FAILED(status)) goto err;

    encoder->outfd       = tty->fd;
    encoder->pixelwidth  = width;
    encoder->pixelheight = height;
    encoder->clipx       = shift_x;
    encoder->clipy       = shift_y;
    encoder->clipwidth   = view_w;
    encoder->clipheight  = view_h;

    status = sixel_encoder_encode(encoder, file);
    if (SIXEL_FAILED(status)) goto err;

    status = SIXEL_OK;
err:
    sixel_encoder_unref(encoder);
}


static void
w3m_stop(void)
{
    logging(DEBUG, "w3m_stop()\n");
    printf("\n");
}


static void
w3m_sync(void)
{
    logging(DEBUG, "w3m_sync()\n");
    printf("\n");
}


static void
w3m_nop(void)
{
    logging(DEBUG, "w3m_nop()\n");
    printf("\n");
}


static SIXELSTATUS
cb(sixel_frame_t *frame, void *data)
{
    sixel_callback_context_for_getsize_t *callback_context;

    callback_context = (sixel_callback_context_for_getsize_t *)data;

    callback_context->width  = sixel_frame_get_width(frame);
    callback_context->height = sixel_frame_get_height(frame);

    return SIXEL_OK;
}


static void
w3m_getsize(struct tty_t const * const tty, const char * const file)
{
    int width, height;
    sixel_callback_context_for_getsize_t cb_context;
    SIXELSTATUS status = SIXEL_FALSE;
    sixel_encoder_t *encoder = NULL;

    logging(DEBUG, "w3m_getsize()\n");

    status = sixel_encoder_new(&encoder, NULL);
    if (SIXEL_FAILED(status)) goto err;

    status = sixel_helper_load_image_file(file,
                                          1,
                                          1,
                                          SIXEL_PALETTE_MAX,
                                          encoder->bgcolor,
                                          SIXEL_LOOP_DISABLE,
                                          cb,
                                          encoder->finsecure,
                                          encoder->cancel_flag,
                                          &cb_context,
                                          encoder->allocator);
    if (SIXEL_FAILED(status)) goto err;

    width  = cb_context.width;
    height = cb_context.height;

    if ((width % tty->cell_width) != 0)
        width  += tty->cell_width - (width % tty->cell_width);
    if ((height % tty->cell_height) != 0)
        height += tty->cell_height - (height % tty->cell_height);

    logging(DEBUG, "image size is %d x %d\n", width, height);

    printf("%d %d\n", width, height);

err:
    if (status != SIXEL_OK)
        printf("%d %d\n", 0, 0);

    sixel_encoder_unref(encoder);
}


static void
w3m_clear(struct tty_t const * const tty, struct parm_t const * const parm)
{
    char buf[BUFSIZE];

    logging(DEBUG, "w3m_clear()\n");

    int offset_x  = str2num(parm->argv[1]);
    int offset_y  = str2num(parm->argv[2]);
    int width     = str2num(parm->argv[3]);
    int height    = str2num(parm->argv[4]);

    /* cursor move */
    snprintf(buf,
             BUFSIZE,
             "\033[%d;%dH\033[;%d;%d;%d;%d$x",
             (offset_y / tty->cell_height),
             (offset_x / tty->cell_width),
             offset_y / tty->cell_height,
             offset_x / tty->cell_width,
             (offset_y + height) / tty->cell_height,
             (offset_x + width) / tty->cell_width);
    ewrite(tty->fd, buf, strlen(buf));
}


#if HAVE_SIGNAL
static void
sig_handler(int signo)
{
    extern volatile sig_atomic_t window_resized; /* global */

    if (signo == SIGWINCH)
        window_resized = true;
}
#endif


static void
set_terminal_size(
    struct tty_t * const /* in/out */ tty,
    int                  /* in */     width,
    int                  /* in */     height,
    int                  /* in */     cols,
    int                  /* in */     lines)
{
    tty->cell_width  = (width / cols);
    tty->cell_height = (height / lines);

    tty->width  = width;
    tty->height = height;
}


static int
check_fds(fd_set *fds, struct timeval * const tv, int fd)
{
    FD_ZERO(fds);
    FD_SET(fd, fds);
    tv->tv_sec  = 0;
    tv->tv_usec = SELECT_TIMEOUT;

    return eselect(fd + 1, fds, NULL, NULL, tv);
}


static bool
terminal_query(
    int                /* in */  ttyfd,
    int *              /* out */ height,
    int *              /* out */ width,
    const char * const /* in */  send_seq,
    const char * const /* in */  recv_format)
{
    int ret, check_count;
    char buf[BUFSIZE], *ptr;
    ssize_t size, left, length;
    struct timeval tv;
    fd_set fds;

    length = strlen(send_seq);
    if ((size = ewrite(ttyfd, send_seq, length)) != length) {
        logging(DEBUG, "write error (data:%d != wrote:%d)\n", size, length);
        return false;
    }

    ptr = buf;
    left = BUFSIZE - 1;

    check_count = 0;
    while (check_count < SELECT_CHECK_LIMIT) {
        if ((ret = check_fds(&fds, &tv, ttyfd)) < 0)
            continue;

        if (FD_ISSET(ttyfd, &fds)) {
            /* FIXME: read is blocked!!! */
            if ((size = read(ttyfd, ptr, left)) > 0) {
                *(ptr + size) = '\0';
                logging(DEBUG, "buf: %s\n", buf);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
                if (sscanf(buf, recv_format, height, width) == 2)
                    return true;
#pragma GCC diagnostic pop
            }
            ptr  += size;
            left -= size;
        }
        check_count++;
    }
    return false;
}


static bool
get_tty(struct tty_t * const /* in/out */ tty)
{
    char ttyname[L_ctermid];

    /* get ttyname and open it */
    if (!ctermid(ttyname)
        || (tty->fd = eopen(ttyname, O_RDWR)) < 0) {
        logging(ERROR, "couldn't open controlling terminal\n");
        return false;
    }
    logging(DEBUG, "ttyname:%s fd:%d\n", ttyname, tty->fd);

    return true;
}


static bool
check_terminal_size(struct tty_t * const /* in/out */ tty)
{
    int width, height, cols, lines;
    struct winsize ws;

    /* at first, we try to get pixel size from "struct winsize" */
    if (ioctl(tty->fd, TIOCGWINSZ, &ws)) {
        logging(ERROR, "ioctl: TIOCGWINSZ failed\n");
    } else if (ws.ws_xpixel == 0 || ws.ws_ypixel == 0 || ws.ws_xpixel == UINT16_MAX || ws.ws_ypixel == UINT16_MAX) {
        logging(ERROR, "struct winsize has no pixel information\n");
    } else {
        set_terminal_size(tty, ws.ws_xpixel, ws.ws_ypixel, ws.ws_col, ws.ws_row);
        logging(DEBUG, "width:%d height:%d cols:%d lines:%d\n",
            ws.ws_xpixel, ws.ws_ypixel, ws.ws_col, ws.ws_row);
        logging(DEBUG, "terminal size set by winsize\n");
        return true;
    }

    /* second, try term sequence:
        CSI 14 t: request window size in pixels.
            -> responce: CSI 4 ; height ; width t
        CSI 18 t: request text area size in characters
            -> responce: CSI 8 ; height ; width t
    */
    if (terminal_query(tty->fd, &height, &width, "\033[14t", "\033[4;%d;%dt")
        && terminal_query(tty->fd, &lines, &cols, "\033[18t", "\033[8;%d;%dt")) {
        set_terminal_size(tty, width, height, cols, lines);
        logging(DEBUG, "width:%d height%d cols:%d lines:%d\n", width, height, cols, lines);
        logging(DEBUG, "terminal size set by dtterm sequence\n");
        return true;
    } else {
        logging(ERROR, "no responce for dtterm sequence\n");
    }

    /* finally, use default values */
    set_terminal_size(tty, TERM_WIDTH, TERM_HEIGHT,
        TERM_WIDTH / CELL_WIDTH, TERM_HEIGHT / CELL_HEIGHT);
    logging(DEBUG, "terminal size set by default value\n");

    return true;
}


int
main(int argc, char *argv[])
{
    /*
    w3mimg protocol
      0  1  2 ....
     +--+--+--+--+ ...... +--+--+
     |op|; |args             |\n|
     +--+--+--+--+ .......+--+--+

     args is separeted by ';'
     op   args
      0;  params    draw image
      1;  params    redraw image
      2;  -none-    terminate drawing
      3;  -none-    sync drawing
      4;  -none-    nop, sync communication
                    response '\n'
      5;  path      get size of image,
                    response "<width> <height>\n"
      6;  params(6) clear image

      params
       <n>;<x>;<y>;<w>;<h>;<sx>;<sy>;<sw>;<sh>;<path>
      params(6)
       <x>;<y>;<w>;<h>
    */
    int i, op, optind;
    char buf[BUFSIZE], *cp;
    struct tty_t tty;
    struct parm_t parm;
    struct winsize ws;

    if (freopen(log_file, "a", stderr) == NULL)
        logging(ERROR, "freopen (stderr to %s) faild\n", log_file);

    logging(DEBUG, "--- new instance ---\n");
    for (i = 0; i < argc; i++)
        logging(DEBUG, "argv[%d]:%s\n", i, argv[i]);
    logging(DEBUG, "argc:%d\n", argc);

#if HAVE_SIGNAL
    /* register signal handler for window resize event */
    struct sigaction sigact = {
        .sa_handler = sig_handler,
        .sa_flags   = SA_RESTART,
    };
    esigaction(SIGWINCH, &sigact, NULL);
#endif

    if (!get_tty(&tty))
        goto release;

    if (!check_terminal_size(&tty))
        goto release;

    logging(DEBUG, "terminal size width:%d height:%d cell_width:%d cell_height:%d\n",
        tty.width, tty.height, tty.cell_width, tty.cell_height);

    /* check args */
    optind = 1;
    while (optind < argc) {
        if (strncmp(argv[optind], "-test", 5) == 0) {
            printf("%d %d\n", tty.width, tty.height);
            logging(DEBUG, "responce: %d %d\n", tty.width, tty.height);
            goto release;
        }
        else if (strncmp(argv[optind], "-size", 5) == 0 && ++optind < argc) {
            w3m_getsize(&tty, argv[optind]);
            goto release;
        }
        optind++;
    }

    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    /* main loop */
    while (fgets(buf, BUFSIZE, stdin) != NULL) {
        if (window_resized) {
            window_resized = false;
            if (ioctl(tty.fd, TIOCGWINSZ, &ws)) {
                logging(ERROR, "ioctl: TIOCGWINSZ failed\n");
                set_terminal_size(&tty, TERM_WIDTH, TERM_HEIGHT,
                    CELL_WIDTH, CELL_HEIGHT);
            } else {
                set_terminal_size(&tty, CELL_WIDTH * ws.ws_col,
                    CELL_HEIGHT * ws.ws_row, ws.ws_col, ws.ws_row);
            }
            logging(DEBUG, "window resized!\n");
            logging(DEBUG, "terminal size width:%d height:%d cell_width:%d cell_height:%d\n",
                tty.width, tty.height, tty.cell_width, tty.cell_height);
        }

        if ((cp = strchr(buf, '\n')) != NULL)
            *cp = '\0';
        logging(DEBUG, "stdin: %s\n", buf);

        reset_parm(&parm);
        parse_arg(buf, &parm, ';', isgraph);

        if (parm.argc <= 0)
            continue;

        op = str2num(parm.argv[0]);
        if (op < 0 || op >= NUM_OF_W3M_FUNC)
            continue;

        switch (op) {
            case W3M_DRAW:
            case W3M_REDRAW:
                w3m_draw(&tty, &parm, op);
                break;
            case W3M_STOP:
                w3m_stop();
                break;
            case W3M_SYNC:
                w3m_sync();
                break;
            case W3M_NOP:
                w3m_nop();
                break;
            case W3M_GETSIZE:
                if (parm.argc != 2)
                    break;
                w3m_getsize(&tty, parm.argv[1]);
                break;
            case W3M_CLEAR:
                w3m_clear(&tty, &parm);
                break;
            default:
                break;
        }
    }

release:
    fflush(stdout);
    fflush(stderr);

    efclose(stderr);

    if (tty.fd > 0)
        eclose(tty.fd);

    logging(DEBUG, "exiting...\n");
    return EXIT_SUCCESS;
}

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
