#pragma once

#include "./X_Base.h"

X_CNAME_BEGIN

X_API int     X_opterr; /* if error message should be printed */
X_API int     X_optind; /* index into parent argv vector */
X_API int     X_optopt; /* character checked for validity */
X_API int     X_optreset; /* reset getopt */
X_API char *  X_optarg;   /* argument associated with option */

X_API int X_getopt(int nargc, char *const nargv[], const char *ostr);

X_CNAME_END
