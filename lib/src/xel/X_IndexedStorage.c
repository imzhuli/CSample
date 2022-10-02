#include <xel/X_IndexedStorage.h>
#include <xel/X_Chrono.h>

#define InvalidIndex    ((uint32_t)-1)
#define MaxIndexValue   ((uint32_t)0x7FFFFFFFu)
#define KeyInUseBitmask ((uint32_t)0x8000000u)

bool XIP_Init(XelIndexIdPool * PoolPtr, size_t Size)
{
	assert(Size && Size < MaxIndexValue);

	XelIndexIdPool InitObject = { ._NextFreeIdIndex = XelInvalidIndexId };
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

	XelIndexIdPool InitObject = { ._NextFreeIdIndex = XelInvalidIndexId };
	*PoolPtr = InitObject;
}

XelIndexId XIP_Acquire(XelIndexIdPool * PoolPtr)
{
	uint32_t Index;
	if (PoolPtr->_NextFreeIdIndex == InvalidIndex) {
		if (X_UNLIKELY(PoolPtr->_InitedId >= PoolPtr->_IdPoolSize)) {
			return XelInvalidIndexId;
		}
		Index = PoolPtr->_InitedId++;
	} else {
		Index = PoolPtr->_NextFreeIdIndex;
		PoolPtr->_NextFreeIdIndex = PoolPtr->_IdPoolPtr[Index];
	}
	uint32_t Rand = ++PoolPtr->_Counter | KeyInUseBitmask;
	PoolPtr->_IdPoolPtr[Index] = Rand;
	return (((XelIndexId)Rand) << 32) + Index;
}

void XIP_Release(XelIndexIdPool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	PoolPtr->_IdPoolPtr[Index] = PoolPtr->_NextFreeIdIndex;
	PoolPtr->_NextFreeIdIndex = Index;
}

bool XIP_Check(XelIndexIdPool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	return X_LIKELY(Key & KeyInUseBitmask) && X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index]);
}

bool XIP_CheckAndRelease(XelIndexIdPool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize) ) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	if(!X_LIKELY(Key & KeyInUseBitmask) || !X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index])) {
		return false;
	}
	PoolPtr->_IdPoolPtr[Index] = PoolPtr->_NextFreeIdIndex;
	PoolPtr->_NextFreeIdIndex = Index;
	return true;
}

////// Storage

bool XISP_Init(XelIndexedStoragePool * PoolPtr, size_t Size)
{
	assert(Size && Size < MaxIndexValue);

	XelIndexedStoragePool InitObject = { ._NextFreeIdIndex = XelInvalidIndexId };
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

	XelIndexedStoragePool InitObject = { ._NextFreeIdIndex = XelInvalidIndexId };
	*PoolPtr = InitObject;
}

XelIndexId XISP_Acquire(XelIndexedStoragePool * PoolPtr, XelVariable Value)
{
	uint32_t Index;
	struct XelIndexedStoragePool_Node * NodePtr;
	if (PoolPtr->_NextFreeIdIndex == InvalidIndex) {
		if (X_UNLIKELY(PoolPtr->_InitedId >= PoolPtr->_IdPoolSize)) {
			return XelInvalidIndexId;
		}
		NodePtr = &PoolPtr->_IdPoolPtr[(Index = PoolPtr->_InitedId++)];
	} else {
		NodePtr = &PoolPtr->_IdPoolPtr[(Index = PoolPtr->_NextFreeIdIndex)];
		PoolPtr->_NextFreeIdIndex = NodePtr->Index;
	}
	uint32_t Rand = ++PoolPtr->_Counter | KeyInUseBitmask;
	NodePtr->Key = Rand;
	NodePtr->Value = Value;
	return (((XelIndexId)Rand) << 32) + Index;
}

void XISP_Release(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	PoolPtr->_IdPoolPtr[Index].Index = PoolPtr->_NextFreeIdIndex;
	PoolPtr->_NextFreeIdIndex = Index;
}

bool XISP_Check(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return false;
	}
	uint32_t Key = XII_ExtractKey(Id);
	return X_LIKELY(Key & KeyInUseBitmask) && X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index].Key);
}

XelOptionalVariable XISP_CheckAndGet(XelIndexedStoragePool * PoolPtr, XelIndexId Id)
{
	uint32_t Index = XII_ExtractIndex(Id);
	if (!X_LIKELY(Index < PoolPtr->_IdPoolSize)) {
		return XOV_NoValue();
	}
	struct XelIndexedStoragePool_Node * NodePtr = &PoolPtr->_IdPoolPtr[Index];
	uint32_t Key = XII_ExtractKey(Id);
	if (X_LIKELY(Key & KeyInUseBitmask) && X_LIKELY(Key == NodePtr->Key)) {
		return XOV_Value(NodePtr->Value);
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
	if (X_LIKELY(Key & KeyInUseBitmask) && X_LIKELY(Key == NodePtr->Key)) {
		return &NodePtr->Value;
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
	if(!X_LIKELY(Key & KeyInUseBitmask) || !X_LIKELY(Key == PoolPtr->_IdPoolPtr[Index].Key)) {
		return false;
	}
	PoolPtr->_IdPoolPtr[Index].Index = PoolPtr->_NextFreeIdIndex;
	PoolPtr->_NextFreeIdIndex = Index;
	return true;
}
