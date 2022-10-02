#include <xel/X_IndexedStorage.h>
#include <xel/X_Chrono.h>

#define NoFreeIndex     ((uint32_t)-1)
#define MaxIndexValue   ((uint32_t)0x3FFFFFFFu)
#define KeyInUseBitmask ((uint32_t)0x80000000u)
#define KeyMask         (MaxIndexValue | KeyInUseBitmask)
X_STATIC_INLINE bool IsSafeKey(uint32_t Key) { return X_LIKELY(Key != (uint32_t)-1); }

bool XIP_Init(XelIndexIdPool * PoolPtr, size_t Size)
{
	assert(Size && Size < MaxIndexValue);

	XelIndexIdPool InitObject = { ._NextFreeIndex = NoFreeIndex };
	*PoolPtr = InitObject;

	if (!(PoolPtr->_IdPoolPtr = (uint32_t*)malloc(sizeof(uint32_t) * Size))) {
		return false;
	}
	PoolPtr->_IdPoolSize = (uint32_t)Size;
	PoolPtr->_Counter = (uint32_t)X_GetTimestampUS();
	return true;
}

void XIP_Clean(XelIndexIdPool * PoolPtr)
{
	assert(PoolPtr && PoolPtr->_IdPoolPtr);
	free(PoolPtr->_IdPoolPtr);

	XelIndexIdPool InitObject = { ._NextFreeIndex = NoFreeIndex };
	*PoolPtr = InitObject;
}

XelIndexId XIP_Acquire(XelIndexIdPool * PoolPtr)
{
	uint32_t Index;
	if (PoolPtr->_NextFreeIndex == NoFreeIndex) {
		if (X_UNLIKELY(PoolPtr->_InitedId >= PoolPtr->_IdPoolSize)) {
			return XelInvalidIndexId;
		}
		Index = PoolPtr->_InitedId++;
	} else {
		Index = PoolPtr->_NextFreeIndex;
		PoolPtr->_NextFreeIndex = PoolPtr->_IdPoolPtr[Index];
	}
	uint32_t Rand = ++PoolPtr->_Counter;
	Rand |= KeyInUseBitmask;
	Rand &= KeyMask;
	PoolPtr->_IdPoolPtr[Index] = Rand;
	return (((XelIndexId)Rand) << 32) + Index;
}

void XIP_Release(XelIndexIdPool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	PoolPtr->_IdPoolPtr[Index] = PoolPtr->_NextFreeIndex;
	PoolPtr->_NextFreeIndex = Index;
}

bool XIP_Check(XelIndexIdPool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	return X_LIKELY(IsSafeKey(Key)) && X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index]);
}

bool XIP_CheckAndRelease(XelIndexIdPool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize) ) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	if(!X_LIKELY(IsSafeKey(Key)) || !X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index])) {
		return false;
	}
	PoolPtr->_IdPoolPtr[Index] = PoolPtr->_NextFreeIndex;
	PoolPtr->_NextFreeIndex = Index;
	return true;
}

////// Storage

bool XISP_Init(XelIndexedStoragePool * PoolPtr, size_t Size)
{
	assert(Size && Size < MaxIndexValue);

	XelIndexedStoragePool InitObject = { ._NextFreeIndex = NoFreeIndex };
	*PoolPtr = InitObject;

	if (!(PoolPtr->_IdPoolPtr = (struct XelIndexedStoragePool_Node*)malloc(sizeof(struct XelIndexedStoragePool_Node) * Size))) {
		return false;
	}
	PoolPtr->_IdPoolSize = (uint32_t)Size;
	PoolPtr->_Counter = (uint32_t)X_GetTimestampUS();
	return true;
}

void XISP_Clean(XelIndexedStoragePool * PoolPtr)
{
	assert(PoolPtr && PoolPtr->_IdPoolPtr);
	free(PoolPtr->_IdPoolPtr);

	XelIndexedStoragePool InitObject = { ._NextFreeIndex = NoFreeIndex };
	*PoolPtr = InitObject;
}

XelIndexId XISP_Acquire(XelIndexedStoragePool * PoolPtr, XelVariable Value)
{
	uint32_t Index;
	struct XelIndexedStoragePool_Node * NodePtr;
	if (PoolPtr->_NextFreeIndex == NoFreeIndex) {
		if (X_UNLIKELY(PoolPtr->_InitedId >= PoolPtr->_IdPoolSize)) {
			return XelInvalidIndexId;
		}
		NodePtr = &PoolPtr->_IdPoolPtr[(Index = PoolPtr->_InitedId++)];
	} else {
		NodePtr = &PoolPtr->_IdPoolPtr[(Index = PoolPtr->_NextFreeIndex)];
		PoolPtr->_NextFreeIndex = NodePtr->_NextFreeIndex;
	}
	uint32_t Rand = ++PoolPtr->_Counter;
	Rand |= KeyInUseBitmask;
	Rand &= KeyMask;
	NodePtr->_Key = Rand;
	NodePtr->_Value = Value;
	return (((XelIndexId)Rand) << 32) + Index;
}

void XISP_Release(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	PoolPtr->_IdPoolPtr[Index]._NextFreeIndex = PoolPtr->_NextFreeIndex;
	PoolPtr->_NextFreeIndex = Index;
}

bool XISP_Check(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	return X_LIKELY(IsSafeKey(Key)) && X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index]._Key);
}

XelOptionalVariable XISP_CheckAndGet(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return XOV_NoValue();
	}
	struct XelIndexedStoragePool_Node * NodePtr = &PoolPtr->_IdPoolPtr[Index];
	uint32_t Key = XII_ExtractKey(Id);
	if (X_LIKELY(IsSafeKey(Key)) && X_LIKELY(Key == NodePtr->_Key)) {
		return XOV_Value(NodePtr->_Value);
	}
	return XOV_NoValue();
}

XelVariable * XISP_CheckAndGetRef(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return NULL;
	}
	struct XelIndexedStoragePool_Node * NodePtr = &PoolPtr->_IdPoolPtr[Index];
	uint32_t Key = XII_ExtractKey(Id);
	if (X_LIKELY(IsSafeKey(Key)) && X_LIKELY(Key == NodePtr->_Key)) {
		return &NodePtr->_Value;
	}
	return NULL;
}

bool XISP_CheckAndRelease(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize) ) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	if(!X_LIKELY(IsSafeKey(Key)) || !X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index]._Key)) {
		return false;
	}
	PoolPtr->_IdPoolPtr[Index]._NextFreeIndex = PoolPtr->_NextFreeIndex;
	PoolPtr->_NextFreeIndex = Index;
	return true;
}
