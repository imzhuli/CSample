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

X_STATIC_INLINE void XLN_AppendUnsafe(XelListNode * NodePtr, XelListNode * To) 
{
	XelListNode * Next = To->NextNodePtr;
	To->NextNodePtr = NodePtr;
	Next->PrevNodePtr = NodePtr;
	NodePtr->PrevNodePtr = To;
	NodePtr->NextNodePtr = Next;
}

X_STATIC_INLINE void XLN_Append(XelListNode * NodePtr, XelListNode * To) 
{
	assert(!X_UNLIKELY(XLN_IsLinked(NodePtr)));
	XLN_AppendUnsafe(NodePtr, To);
}

X_STATIC_INLINE void XLN_InsertUnsafe(XelListNode * NodePtr, XelListNode * Before) 
{
	XLN_AppendUnsafe(NodePtr, Before->PrevNodePtr);
}

X_STATIC_INLINE void XLN_Insert(XelListNode * NodePtr, XelListNode * Before) 
{
	assert(!X_UNLIKELY(XLN_IsLinked(NodePtr)));
	XLN_InsertUnsafe(NodePtr, Before);
}

// List header
X_STATIC_INLINE void XL_Init(XelList * ListPtr) 
{
	XLN_Init(&ListPtr->Head);
}

X_STATIC_INLINE bool XL_IsEmpty(XelList * ListPtr) 
{
	return !XLN_IsLinked(&ListPtr->Head);
}

X_STATIC_INLINE void XL_Clean(XelList * ListPtr) 
{
	assert(XL_IsEmpty(ListPtr));
}

X_STATIC_INLINE XelListForwardIterator XL_Begin(XelList * ListPtr)
{
	XelListForwardIterator InitObject = { ListPtr->Head.NextNodePtr, ListPtr->Head.NextNodePtr->NextNodePtr };
	return InitObject;
}

X_STATIC_INLINE XelListForwardIterator XL_End(XelList * ListPtr)
{
	XelListForwardIterator InitObject = { &ListPtr->Head, NULL };
	return InitObject;
}

X_STATIC_INLINE void XL_AddHead(XelList * ListPtr, XelListNode * NodePtr)
{
	XLN_Append(NodePtr, &ListPtr->Head);
}

X_STATIC_INLINE void XL_GrabHead(XelList * ListPtr, XelListNode * NodePtr)
{
	XLN_DetachUnsafe(NodePtr);
	XLN_AppendUnsafe(NodePtr, &ListPtr->Head);
}

X_STATIC_INLINE void XL_AddTail(XelList * ListPtr, XelListNode * NodePtr)
{
	XLN_Append(NodePtr, ListPtr->Head.PrevNodePtr);
}

X_STATIC_INLINE void XL_GrabTail(XelList * ListPtr, XelListNode * NodePtr)
{
	XLN_DetachUnsafe(NodePtr);
	XLN_AppendUnsafe(NodePtr, ListPtr->Head.PrevNodePtr);
}

X_STATIC_INLINE XelListNode * XL_PopHead(XelList * ListPtr)
{
	if (XL_IsEmpty(ListPtr)) {
		return NULL;
	}
	XelListNode * Ret = ListPtr->Head.NextNodePtr;
	XLN_Detach(Ret);
	return Ret;
}

X_STATIC_INLINE XelListNode * XL_PopTail(XelList * ListPtr)
{
	if (XL_IsEmpty(ListPtr)) {
		return NULL;
	}
	XelListNode * Ret = ListPtr->Head.PrevNodePtr;
	XLN_Detach(Ret);
	return Ret;
}

X_STATIC_INLINE void XL_GrabListTail(XelList * ListPtr, XelList * From)
{
	if (XL_IsEmpty(From)) {
		return;
	}
	
	XelListNode * RemoteHead = From->Head.NextNodePtr;
	XelListNode * RemoteTail = From->Head.PrevNodePtr;
	XLN_Init(&From->Head);

	XelListNode * LocalTail = ListPtr->Head.PrevNodePtr;
	ListPtr->Head.PrevNodePtr = RemoteTail;
	RemoteTail->NextNodePtr = &ListPtr->Head;
	LocalTail->PrevNodePtr = RemoteHead;
	RemoteHead->PrevNodePtr = LocalTail;
}

// forward iterator

X_STATIC_INLINE bool XLFI_IsEqual(XelListForwardIterator IterPtr, XelListForwardIterator To)
{
	return IterPtr.CurrentNodePtr == To.CurrentNodePtr;
}

X_STATIC_INLINE XelListNode *XLFI_Get(XelListForwardIterator Iter) 
{
	return Iter.CurrentNodePtr;
}

X_STATIC_INLINE XelListForwardIterator XLFI_Next(XelListForwardIterator Iter)
{
	XelListForwardIterator InitObject = { Iter.NextNodePtr, Iter.NextNodePtr->NextNodePtr };
	return InitObject;
}

X_CNAME_END
