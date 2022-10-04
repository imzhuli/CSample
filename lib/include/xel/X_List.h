#pragma once
#include "./X_Base.h"

X_CNAME_BEGIN

struct XelListNode
{
	struct XelListNode * PrevNodePtr;
	struct XelListNode * NextNodePtr;
};

struct XelListForwardIterator
{
	struct XelListNode * CurrentNodePtr;
	struct XelListNode * NextNodePtr;
};

typedef struct XelListNode XelListNode;
typedef struct XelListForwardIterator XelListForwardIterator;

struct XelList
{
	XelListNode Head;
};
typedef struct XelList XelList;

X_STATIC_INLINE void XLN_Init(XelListNode * NodePtr) 
{
	NodePtr->PrevNodePtr = NodePtr->NextNodePtr = NodePtr;
}

X_STATIC_INLINE bool XLN_IsLinked(XelListNode * NodePtr)
{
	return NodePtr != NodePtr->NextNodePtr;
}

X_STATIC_INLINE void XLN_DetachUnsafe(XelListNode * NodePtr) 
{
	NodePtr->PrevNodePtr->NextNodePtr = NodePtr->NextNodePtr;
	NodePtr->NextNodePtr->PrevNodePtr = NodePtr->PrevNodePtr;
}

X_STATIC_INLINE void XLN_Detach(XelListNode * NodePtr) 
{
	XLN_DetachUnsafe(NodePtr);
	XLN_Init(NodePtr);
}

X_STATIC_INLINE void XLN_Replace(XelListNode * TargetPtr, XelListNode * By) 
{	
	assert(!X_UNLIKELY(XLN_IsLinked(By)));
	By->PrevNodePtr = TargetPtr->PrevNodePtr;
	By->NextNodePtr = TargetPtr->NextNodePtr;
	By->NextNodePtr->PrevNodePtr = By;
	By->PrevNodePtr->NextNodePtr = By;
	XLN_Init(TargetPtr);
}

X_STATIC_INLINE void XLN_Append(XelListNode * NodePtr, XelListNode * To) 
{
	assert(!X_UNLIKELY(XLN_IsLinked(NodePtr)));
	XelListNode * Next = To->NextNodePtr;
	To->NextNodePtr = NodePtr;
	Next->PrevNodePtr = NodePtr;
	NodePtr->PrevNodePtr = To;
	NodePtr->NextNodePtr = Next;
}

// List header
X_STATIC_INLINE void XL_Init(XelList * ListPtr) 
{
	XLN_Init(&ListPtr->Head);
}

X_STATIC_INLINE XelListForwardIterator XL_Begin(XelList * ListPtr)
{
	XelListForwardIterator InitObject = { ListPtr->Head.NextNodePtr, ListPtr->Head.NextNodePtr->NextNodePtr };
	return InitObject;
}

X_STATIC_INLINE XelListForwardIterator XL_End(XelList * ListPtr)
{
	XelListForwardIterator InitObject = { &ListPtr->Head, ListPtr->Head.NextNodePtr };
	return InitObject;
}

// forward iterator

X_STATIC_INLINE XelListForwardIterator XLFI_Next(XelListForwardIterator Iter)
{
	XelListForwardIterator InitObject = { Iter.NextNodePtr, Iter.NextNodePtr->NextNodePtr };
	return InitObject;
}

X_CNAME_END
