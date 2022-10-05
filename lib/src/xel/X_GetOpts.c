/*
 * Copyright (c) 1987, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <xel/X_GetOpts.h>
#include <string.h>
#include <stdio.h>

int X_opterr = 1;         /* if error message should be printed */
int X_optind = 1;         /* index into parent argv vector */
int X_optopt = 0;         /* character checked for validity */
int X_optreset = 0;       /* reset getopt */
char * X_optarg = NULL;   /* argument associated with option */

#define BADCH (int)'?'
#define BADARG (int)':'
#define EMSG ""

/*
 * getopt --
 *      Parse argc/argv argument vector.
 */
int X_getopt(int nargc, char *const nargv[], const char *ostr)
{
    static char *place = EMSG; /* option letter processing */
    const char *oli;           /* option letter list index */

    if (X_optreset || !*place)
    { /* update scanning pointer */
        X_optreset = 0;
        if (X_optind >= nargc || *(place = nargv[X_optind]) != '-')
        {
            place = EMSG;
            return (-1);
        }
        if (place[1] && *++place == '-')
        { /* found "--" */
            ++X_optind;
            place = EMSG;
            return (-1);
        }
    } /* option letter okay? */
    if ((X_optopt = (int)*place++) == (int)':' ||
        !(oli = strchr(ostr, X_optopt)))
    {
        /*
         * if the user didn't specify '-' as an option,
         * assume it means -1.
         */
        if (X_optopt == (int)'-')
            return (-1);
        if (!*place)
            ++X_optind;
        if (X_opterr && *ostr != ':')
            (void)printf("illegal option -- %c\n", X_optopt);
        return (BADCH);
    }
    if (*++oli != ':')
    { /* don't need argument */
        X_optarg = NULL;
        if (!*place)
            ++X_optind;
    }
    else
    {               /* need an argument */
        if (*place) /* no white space */
            X_optarg = place;
        else if (nargc <= ++X_optind)
        { /* no arg */
            place = EMSG;
            if (*ostr == ':')
                return (BADARG);
            if (X_opterr)
                (void)printf("option requires an argument -- %c\n", X_optopt);
            return (BADCH);
        }
        else /* white space */
            X_optarg = nargv[X_optind];
        place = EMSG;
        ++X_optind;
    }
    return (X_optopt); /* dump back option letter */
}
