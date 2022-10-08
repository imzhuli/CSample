#pragma once
#include "../X_Base.h"

X_CNAME_BEGIN

typedef XEL_HANDLE XelIniReader;

X_API XelIniReader XIR_Init(const char * IniFilename);
X_API void XIR_Clean(XelIniReader Reader);
X_API const char * XIR_Get(XelIniReader Reader, const char * Key);
X_API bool XIR_GetBool(XelIniReader Reader, const char *key, bool vxDefaultValue);
X_API int64_t XIR_GetInt64(XelIniReader Reader, const char *key, int64_t vxDefaultValue);


X_CNAME_END
