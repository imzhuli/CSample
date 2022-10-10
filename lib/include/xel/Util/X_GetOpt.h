#pragma once

#include "../X_Base.h"

X_CNAME_BEGIN

X_API int     X_OptErr;   /* if error message should be printed */
X_API int     X_OptInd;   /* index into parent argv vector */
X_API int     X_OptOpt;   /* character checked for validity */
X_API int     X_OptReset; /* reset getopt */
X_API char *  X_OptArg;   /* argument associated with option */

typedef enum
{
    X_NoArgument = 0,
    X_RequiredArgument = 1,
    X_OptionalArgument = 2,
} XelArgumentRequirement;

typedef struct X_LongOption
{
  const char *              name;
  XelArgumentRequirement    has_arg;
  int *                     flag;
  int                       val;
} X_LongOption;

X_API int X_GetOpt(int nargc, char *const nargv[], const char *ostr);
X_API int X_GetOptLong(int nargc, char *nargv[], const char * options, X_LongOption * long_options, int * index);

X_CNAME_END
