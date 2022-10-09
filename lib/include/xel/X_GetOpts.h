#pragma once

#include "./X_Base.h"

X_CNAME_BEGIN

X_API int     X_OptErr;   /* if error message should be printed */
X_API int     X_OptInd;   /* index into parent argv vector */
X_API int     X_OptOpt;   /* character checked for validity */
X_API int     X_OptReset; /* reset getopt */
X_API char *  X_OptArg;   /* argument associated with option */

X_API int X_GetOpt(int nargc, char *const nargv[], const char *ostr);

X_CNAME_END
