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

#ifndef LIBSIXEL_CONVERTERS_PARSEARG_H
#define LIBSIXEL_CONVERTERS_PARSEARG_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MAX_ARGS         = 128,
};

struct parm_t {
    int argc;
    char *argv[MAX_ARGS];
};

static void
reset_parm(struct parm_t *pt)
{
    int i;

    pt->argc = 0;
    for (i = 0; i < MAX_ARGS; i++)
        pt->argv[i] = NULL;
}

static void
add_parm(struct parm_t *pt, char *cp)
{
    if (pt->argc >= MAX_ARGS)
        return;

    pt->argv[pt->argc] = cp;
    pt->argc++;
}

static void
parse_arg(char *buf, struct parm_t *pt, int delim, int (is_valid)(int c))
{
    int i, length;
    char *cp, *vp;

    if (buf == NULL)
        return;

    length = strlen(buf);

    vp = NULL;
    for (i = 0; i < length; i++) {
        cp = &buf[i];

        if (vp == NULL && is_valid(*cp))
            vp = cp;

        if (*cp == delim) {
            *cp = '\0';
            add_parm(pt, vp);
            vp = NULL;
        }

        if (i == (length - 1) && (vp != NULL || *cp == '\0'))
            add_parm(pt, vp);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* LIBSIXEL_CONVERTERS_PARSEARG_H */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
