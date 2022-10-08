#include <xel/Util/X_IniRead.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if defined(X_SYSTEM_WINDOWS)
#define strcasecmp _stricmp
#elif defined(X_SYSTEM_MACOS)
#include <strings.h> // mac: strcasecmp
#endif

typedef struct __IniFileLine__
{
    size_t xLineNumber;
    char * hKeyStart;
    char * hValueStart;
} __IniFileLine__;

typedef struct __IniContent__
{
    __IniFileLine__ *    pLines;
    size_t               xLineCount;
    size_t               xFileSize;
    char                 xFileContent[1];
} __IniContent__;

static void XIR_IniProcessLine(__IniFileLine__ * vhLine)
{
    // always assume the line ends with '\0'

    char * vhAssignChar = vhLine->hKeyStart;
    char * vhValueStart = NULL;
    if (*vhAssignChar == '#') { // treat comment as empty line
        *vhAssignChar = '\0';
    }
    if (*vhAssignChar == '\0') { // empty line
        vhLine->hValueStart = vhAssignChar;
        return;
    }

    do {
        ++vhAssignChar;
        if (*vhAssignChar == '\0') {
            vhValueStart = vhAssignChar;
            break;
        }
        if (*vhAssignChar == '=') {
            *vhAssignChar = '\0';
            vhValueStart = vhAssignChar + 1;
            break;
        }
    } while(true);

    // r-Trim key:
    --vhAssignChar;
    while(vhAssignChar != vhLine->hKeyStart && isspace(*vhAssignChar)) {
        *(vhAssignChar--) = '\0';
    }

    // l-Trim value
    while(*vhValueStart && isspace(*vhValueStart)) {
        ++vhValueStart;
    }
    vhLine->hValueStart = vhValueStart;
}

static int XIR_IniKeyCompare(const void * vhLeft, const void * vhRight)
{
    __IniFileLine__ * vhLeftKey = (__IniFileLine__ *)vhLeft;
    __IniFileLine__ * vhRightKey = (__IniFileLine__ *)vhRight;
    int vhCompareResult =  strcmp(vhLeftKey->hKeyStart, vhRightKey->hKeyStart);
    if (!vhCompareResult) {
        return (int)(vhLeftKey->xLineNumber - vhRightKey->xLineNumber);
    }
    return vhCompareResult;
}

static bool XIR_IniPreprocess(__IniContent__ * vhIni)
{
    // count lines:
    char * vhCurr = vhIni->xFileContent;
    char * vhEnd = vhIni->xFileContent + vhIni->xFileSize;
    size_t vxLineCount = 0;
    if(vhCurr < vhEnd) {
        do {
            if (vhCurr[0] == '\r' && vhCurr[1]) {
                // replace \r\n with "\n " (a newline and a space)
                // it is guaranteed that file content ends with '\0',
                // so accessing vhCurr[1] never causes overflow.
                // the space will be removed in later procedures
                vhCurr[0] = '\n';
                vhCurr[1] = ' ';
                ++vxLineCount;
                ++vhCurr; // Skip original \r;
            }
            else if (vhCurr[0] == '\n') {
                ++vxLineCount;
            }
            ++vhCurr;
        } while(vhCurr < vhEnd);
        ++vxLineCount;
    }
    // don't forget the last line, even if it might be empty;
    if (!(vhIni->pLines = (__IniFileLine__*)malloc(sizeof(__IniFileLine__) * vxLineCount + 1))) {
        return false;
    }
    vhIni->xLineCount = vxLineCount;

    // Split file into lines:
    vhCurr = vhIni->xFileContent;
    for (size_t i = 0 ; i < vxLineCount; ++i) {
        __IniFileLine__ * vhLine = vhIni->pLines + i;

        // Skip spaces:
        while(*vhCurr) {
            if (!isspace(*vhCurr)) {
                break;
            }
            ++vhCurr;
        }

        char * vhKeyStart = vhCurr;
        // get to next line:
        while(*vhCurr) {
            if(*vhCurr == '\n') {
                *vhCurr = '\0';
                ++vhCurr;
                break;
            }
            ++vhCurr;
        }
        char * vhLineEnd = vhCurr - 2;
        // r-Trim line
        while(vhLineEnd >= vhKeyStart && isspace(*vhLineEnd)) {
            *(vhLineEnd--) = '\0';
        }
        vhLine->xLineNumber = i;
        vhLine->hKeyStart = vhKeyStart;
        XIR_IniProcessLine(vhLine);
    }

    return true;
}

static void XIR_IniSort(__IniContent__ * vhIni)
{
    qsort(vhIni->pLines, vhIni->xLineCount, sizeof(__IniFileLine__), &XIR_IniKeyCompare);
    // combine:
    __IniFileLine__ * vhLine = vhIni->pLines;
    __IniFileLine__ * vhReplaceLine = vhLine + 1;
    __IniFileLine__ * vhEndLine = vhLine + vhIni->xLineCount;

    // printf("original line count=%d\n", (int)vhIni->xLineCount);
    while(vhReplaceLine < vhEndLine) {
        if (!strcmp(vhLine->hKeyStart, vhReplaceLine->hKeyStart)) {
            *vhLine = *(vhReplaceLine++);
        }
        else {
            *(++vhLine) = *(vhReplaceLine++);
        }
    }
    vhIni->xLineCount = vhLine - vhIni->pLines + 1;
}

static __IniContent__ * XIR_IniRead(const char * filename)
{
    FILE * fp = fopen(filename, "rb");
    size_t vxFileSize = 0;

    __IniContent__ * vpRet = NULL;
    size_t vxIniSize = 0;

    if (!fp) {
        goto LABEL_FILE_OPEN_ERROR;
    }
    if (fseek(fp, 0, SEEK_END)) {
        goto LABEL_FILE_SEEK_ERROR;
    }
    vxFileSize = ftell(fp);
    if (vxFileSize == (size_t)-1L) {
        goto LABEL_INVALID_FILE_SIZE_ERROR;
    }
    fseek(fp, 0, SEEK_SET);

    vxIniSize = sizeof(__IniContent__) + vxFileSize;
    assert(vxIniSize > vxFileSize);

    vpRet = (__IniContent__ *)malloc(vxIniSize);
    if (!vpRet) {
        goto LABEL_ALLOC_RETURN_OBJECT_ERROR;
    }
    if (vxFileSize && fread(vpRet->xFileContent, vxFileSize, 1, fp) != 1) {
        goto LABEL_FILE_READ_ERROR;
    }
    vpRet->xFileContent[vxFileSize] = '\0';
    vpRet->xFileSize = vxFileSize;
    vpRet->xLineCount = 0;
    vpRet->pLines = NULL;
    if (!XIR_IniPreprocess(vpRet)) {
        goto LABEL_PREPROCESS_ERROR;
    }
    XIR_IniSort(vpRet);

    fclose(fp);
    return vpRet;

LABEL_PREPROCESS_ERROR:
LABEL_FILE_READ_ERROR:
    free(vpRet);
LABEL_ALLOC_RETURN_OBJECT_ERROR:
LABEL_INVALID_FILE_SIZE_ERROR:
LABEL_FILE_SEEK_ERROR:
    fclose(fp);
LABEL_FILE_OPEN_ERROR:
    return NULL;
}

X_STATIC_INLINE int XIR_IniSearchCompare(const void* vhkey, const void* vhBlock)
{
    __IniFileLine__ * vhLine = (__IniFileLine__ *)vhBlock;
    return strcmp((const char *)vhkey, vhLine->hKeyStart);
}

X_STATIC_INLINE const char * XIR_IniGet(const __IniContent__ * vhIni, const char * vhKey)
{
    __IniFileLine__ * vhLine = (__IniFileLine__ *)bsearch(vhKey, vhIni->pLines, vhIni->xLineCount, sizeof(__IniFileLine__), &XIR_IniSearchCompare);
    if (!vhLine) {
        return NULL;
    }
    return vhLine->hValueStart;
}

X_STATIC_INLINE void XIR_IniFree(__IniContent__ * vhIni)
{
    if (vhIni->pLines) {
        free(vhIni->pLines);
    }
    free(vhIni);
}

XelIniReader XIR_Init(const char * IniFilename)
{
    return (XelIniReader)XIR_IniRead(IniFilename);
}

void XIR_Clean(XelIniReader Reader)
{
    assert(Reader);
    XIR_IniFree((__IniContent__*)Reader);

}

const char * XIR_Get(XelIniReader Reader, const char * Key)
{
    assert(Reader);
    __IniContent__ * ContentPtr = (__IniContent__*)Reader;
    return XIR_IniGet(ContentPtr, Key);
}

bool XIR_GetBool(XelIniReader Reader, const char *Key, bool vxDefaultValue)
{
    const char * vxStrVal = XIR_Get(Reader, Key);
    if (!vxStrVal) {
        return vxDefaultValue;
    }
    if(0 == strcasecmp(vxStrVal, "true") || 0 == strcasecmp(vxStrVal, "yes")) {
        return true;
    }
    else if (0 == strcasecmp(vxStrVal, "false") || 0 == strcasecmp(vxStrVal, "no")) {
        return false;
    }
    return vxDefaultValue;
}

int64_t XIR_GetInt64(XelIniReader Reader, const char *Key, int64_t vxDefaultValue)
{
	const char * vxStrVal = XIR_Get(Reader, Key);
	if (!vxStrVal) {
		return vxDefaultValue;
	}
	return (int64_t)atoll(vxStrVal);
}
