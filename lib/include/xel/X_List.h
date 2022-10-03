#pragma once
#include "./X_Base.h"

X_CNAME_BEGIN

struct XelListNode
{
	struct XelListNode * PrevNodePtr;
	struct XelListNode * NextNodePtr;
};
typedef struct XelListNode XelListNode;
typedef struct XelListNode XelListHead;

void XLN_Init(XelListNode * NodePtr) {
	NodePtr->PrevNodePtr = NodePtr->NextNodePtr = NodePtr;
}



X_CNAME_END
