#pragma once
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __XelStringHolder { char __PlaceHolder__ [1]; };
typedef struct __XelStringHolder * XelString;
typedef struct __XelStringHolder const * XelConstString;

extern XelString      XS_New(void);
extern XelString      XS_Duplicate(XelConstString Str);
extern XelString      XS_NewData(const char * SourcePtr, size_t Length);
extern XelString      XS_NewString(const char * StringPtr);
extern void           XS_Clear(XelString Str);
extern void           XS_Delete(XelString Str);
extern const char *   XS_GetData(XelConstString Str);
extern size_t         XS_GetLength(XelConstString Str);
extern void           XS_SetData(XelString Str, const char * SourcePtr, size_t Length);
extern void           XS_SetString(XelString Str, const char * StringPtr);
extern void           XS_Append(XelString Str, XelConstString TailStr);
extern void           XS_AppendData(XelString Str, const char * SourcePtr, size_t Length);
extern void           XS_AppendString(XelString Str, const char * StringPtr);
extern XelString      XS_SubString(XelConstString Str, size_t Index, size_t Length);
extern XelString      XS_Concat(XelConstString Str1, XelConstString Str2);
extern XelString      XS_HexShow(const void * DataPtr, size_t Length, bool NeedHeader);
extern void           XS_PrintHexShow(FILE * stream, const void * DataPtr, size_t Length, bool NeedHeader);
extern void           XS_StrToHex(void * dst, const void * str, size_t len);
extern void           XS_HexToStr(void * dst, const void * str, size_t len);

#ifdef __cplusplus
}
#endif
