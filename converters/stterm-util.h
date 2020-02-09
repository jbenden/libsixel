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

#ifndef LIBSIXEL_CONVERTERS_STTERM_UTIL_H
#define LIBSIXEL_CONVERTERS_STTERM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

enum loglevel_t {
    DEBUG = 0,
    WARN,
    ERROR,
    FATAL,
};

static void
logging(enum loglevel_t loglevel, char *format, ...)
{
    va_list arg;
    static const char *loglevel2str[] = {
        [DEBUG] = "DEBUG",
        [WARN]  = "WARN",
        [ERROR] = "ERROR",
        [FATAL] = "FATAL",
    };

    if (loglevel == DEBUG && !VERBOSE)
        return;

    fprintf(stderr, ">>%s<<\t", loglevel2str[loglevel]);
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
}

/* wrapper of C functions */
static inline int
eopen(const char *path, int flag)
{
    int fd;
    errno = 0;

    if ((fd = open(path, flag)) < 0) {
        logging(ERROR, "couldn't open \"%s\"\n", path);
        logging(ERROR, "open: %s\n", strerror(errno));
    }
    return fd;
}

static inline int
eclose(int fd)
{
    int ret = 0;
    errno = 0;

    if ((ret = close(fd)) < 0)
        logging(ERROR, "close: %s\n", strerror(errno));

    return ret;
}

static inline FILE *
efopen(const char *path, char *mode)
{
    FILE *fp;
    errno = 0;

    if ((fp = fopen(path, mode)) == NULL) {
        logging(ERROR, "couldn't open \"%s\"\n", path);
        logging(ERROR, "fopen: %s\n", strerror(errno));
    }
    return fp;
}

static inline int
efclose(FILE *fp)
{
    int ret;
    errno = 0;

    if ((ret = fclose(fp)) < 0)
        logging(ERROR, "fclose: %s\n", strerror(errno));

    return ret;
}

static inline void *
ecalloc(size_t nmemb, size_t size)
{
    void *ptr;
    errno = 0;

    if ((ptr = calloc(nmemb, size)) == NULL)
        logging(ERROR, "calloc: %s\n", strerror(errno));

    return ptr;
}

static inline long int
estrtol(const char *nptr, char **endptr, int base)
{
    long int ret;
    errno = 0;

    ret = strtol(nptr, endptr, base);
    if (ret == LONG_MIN || ret == LONG_MAX) {
        logging(ERROR, "strtol: %s\n", strerror(errno));
        return 0;
    }

    return ret;
}

#if 0
static inline int
emkstemp(char *template)
{
    int ret;
    errno = 0;

    if ((ret = mkstemp(template)) < 0) {
        logging(ERROR, "couldn't open \"%s\"\n", template);
        logging(ERROR, "mkstemp: %s\n", strerror(errno));
    }

    return ret;
}
#endif

#ifdef HAVE_SELECT
static inline int
eselect(int max_fd, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *tv)
{
    int ret;
    errno = 0;

    if ((ret = select(max_fd, readfds, writefds, errorfds, tv)) < 0) {
        if (errno == EINTR)
            return eselect(max_fd, readfds, writefds, errorfds, tv);
        else
            logging(ERROR, "select: %s\n", strerror(errno));
    }
    return ret;
}
#endif

static inline ssize_t
ewrite(int fd, const void *buf, size_t size)
{
    ssize_t ret;
    errno = 0;

    if ((ret = write(fd, buf, size)) < 0) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return ewrite(fd, buf, size);
        else
            logging(ERROR, "write: %s\n", strerror(errno));
    } else if (ret < (ssize_t) size) {
        return ret + ewrite(fd, (char *) buf + ret, size - ret);
    }
    return ret;
}

#if HAVE_SIGNAL
static inline int
esigaction(int signo, struct sigaction *act, struct sigaction *oact)
{
    int ret;
    errno = 0;

    if ((ret = sigaction(signo, act, oact)) < 0)
        logging(ERROR, "sigaction: %s\n", strerror(errno));

    return ret;
}
#endif

/*
#ifdef HAVE_STAT
static inline int
estat(const char *restrict path, struct stat *restrict buf)
{
    int ret;
    errno = 0;

    if ((ret = stat(path, buf)) < 0)
        logging(ERROR, "stat: %s\n", strerror(errno));

    return ret;
}
#endif

#ifdef HAVE_MMAP
static inline void *
emmap(void *addr, size_t len, int prot, int flag, int fd, off_t offset)
{
    uint32_t *fp;
    errno = 0;

    if ((fp = (uint32_t *) mmap(addr, len, prot, flag, fd, offset)) == MAP_FAILED)
        logging(ERROR, "mmap: %s\n", strerror(errno));

    return fp;
}

static inline int
emunmap(void *ptr, size_t len)
{
    int ret;
    errno = 0;

    if ((ret = munmap(ptr, len)) < 0)
        logging(ERROR, "munmap: %s\n", strerror(errno));

    return ret;
}
#endif
*/

/* some useful functions */
static inline int
str2num(char *str)
{
    if (str == NULL)
        return 0;

    return estrtol(str, NULL, 10);
}

/*
static inline void
swapint(int *a, int *b)
{
    int tmp = *a;
    *a  = *b;
    *b  = tmp;
}

static inline int
my_ceil(int val, int div)
{
    return (val + div - 1) / div;
}

static inline uint32_t
bit_reverse(uint32_t val, int bits)
{
    uint32_t ret = val;
    int shift = bits - 1;

    for (val >>= 1; val; val >>= 1) {
        ret <<= 1;
        ret |= val & 1;
        shift--;
    }

    return ret <<= shift;
}
*/

#ifdef __cplusplus
}
#endif

#endif /* LIBSIXEL_CONVERTERS_STTERM_UTIL_H */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
