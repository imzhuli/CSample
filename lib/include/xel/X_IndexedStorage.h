#pragma once
#include "./X_Base.h"

X_CNAME_BEGIN

typedef uint64_t XelIndexId;
#define XelInvalidIndexId       ((XelIndexId)0)

typedef struct XelIndexIdPool XelIndexIdPool;

X_API XelIndexIdPool *  XIP_New(size_t Size);
X_API void              XIP_Delete(XelIndexIdPool * PoolPtr);
X_API XelIndexId        XIP_Acquire(XelIndexIdPool * PoolPtr);
X_API void              XIP_Release(XelIndexIdPool * PoolPtr, XelIndexId Id);
X_API bool              XIP_Check(XelIndexIdPool * PoolPtr, XelIndexId Id);
X_API bool              XIP_CheckAndRelease(XelIndexIdPool * PoolPtr, XelIndexId Id);

typedef struct XelIndexedStoragePool XelIndexedStoragePool;

X_API XelIndexedStoragePool *     XISP_New(size_t Size);
X_API void                        XISP_Delete(XelIndexedStoragePool * PoolPtr);
X_API XelIndexId                  XISP_Acquire(XelIndexedStoragePool * PoolPtr, XelVariable Value);
X_API void                        XISP_Release(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API bool                        XISP_Check(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API XelOptionalVariable         XISP_CheckAndGet(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API XelVariable *               XISP_CheckAndGetRef(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API bool                        XISP_CheckAndRelease(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API XelVariable                 XISP_Get(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API XelVariable *               XISP_GetRef(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API void                        XISP_Set(XelIndexedStoragePool * PoolPtr, XelIndexId Id, XelVariable Value);

X_CNAME_END
