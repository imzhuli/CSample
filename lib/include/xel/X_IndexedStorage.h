#pragma once
#include "./X_Base.h"

X_CNAME_BEGIN

typedef uint64_t XelIndexId;
#define XelInvalidIndexId       ((XelIndexId)0)

X_STATIC_INLINE uint32_t XII_ExtractIndex(XelIndexId Id)  { return (uint32_t)Id; }
X_STATIC_INLINE uint32_t XII_ExtractKey(XelIndexId Id) { return (uint32_t)(Id >> 32); }

typedef struct XelIndexIdPool XelIndexIdPool;
struct XelIndexIdPool
{
	uint32_t*     _IdPoolPtr;
	uint32_t      _IdPoolSize;
	uint32_t      _InitedId;
	uint32_t      _NextFreeIdIndex;
	uint32_t      _Counter;
};

X_API bool          XIP_Init(XelIndexIdPool * PoolPtr, size_t Size);
X_API void          XIP_Clean(XelIndexIdPool * PoolPtr);
X_API XelIndexId    XIP_Acquire(XelIndexIdPool * PoolPtr);
X_API void          XIP_Release(XelIndexIdPool * PoolPtr, XelIndexId Id);
X_API bool          XIP_Check(XelIndexIdPool * PoolPtr, XelIndexId Id);
X_API bool          XIP_CheckAndRelease(XelIndexIdPool * PoolPtr, XelIndexId Id);


struct XelIndexedStoragePool_Node
{
	union {
		uint32_t   _NextFreeIndex;
		uint32_t   _Key;
	};
	XelVariable    _Value;
};
typedef struct XelIndexedStoragePool XelIndexedStoragePool;
struct XelIndexedStoragePool
{
	struct XelIndexedStoragePool_Node *  _IdPoolPtr;
	uint32_t                             _IdPoolSize;
	uint32_t                             _InitedId;
	uint32_t                             _NextFreeIdIndex;
	uint32_t                             _Counter;
};
X_API bool                      XISP_Init(XelIndexedStoragePool * PoolPtr, size_t Size);
X_API void                      XISP_Clean(XelIndexedStoragePool * PoolPtr);
X_API XelIndexId                XISP_Acquire(XelIndexedStoragePool * PoolPtr, XelVariable Value);
X_API void                      XISP_Release(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API bool                      XISP_Check(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API XelOptionalVariable       XISP_CheckAndGet(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API XelVariable *             XISP_CheckAndGetRef(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_API bool                      XISP_CheckAndRelease(XelIndexedStoragePool * PoolPtr, XelIndexId Id);
X_STATIC_INLINE XelVariable     XISP_Get(XelIndexedStoragePool * PoolPtr, XelIndexId Id) { return PoolPtr->_IdPoolPtr[XII_ExtractIndex(Id)]._Value; }
X_STATIC_INLINE XelVariable *   XISP_GetRef(XelIndexedStoragePool * PoolPtr, XelIndexId Id) { return &PoolPtr->_IdPoolPtr[XII_ExtractIndex(Id)]._Value; }
X_STATIC_INLINE void            XISP_Set(XelIndexedStoragePool * PoolPtr, XelIndexId Id, XelVariable Value) { *XISP_GetRef(PoolPtr, Id) = Value; }

X_CNAME_END
