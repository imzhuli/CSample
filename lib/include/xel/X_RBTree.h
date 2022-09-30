#pragma once
#include "./X_Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Node */
typedef struct XelRBNode XelRBNode;
struct XelRBNode {
    XelRBNode *   ParentPtr;
    XelRBNode *   LeftNodePtr;
    XelRBNode *   RightNodePtr;
    bool          RedFlag;
};

typedef struct XelRBInsertSlot
{
    XelRBNode * ParentPtr;
    XelRBNode ** SubNodeRefPtr;
} XelRBInsertSlot;

static inline void XRBN_Init(XelRBNode * NodePtr) {
    XelRBNode InitValue = { NULL, NULL, NULL, false };
    *NodePtr = InitValue;
}

static inline bool XRBN_IsRoot(XelRBNode * NodePtr) {
    return !NodePtr->ParentPtr;
}
static inline bool XRBN_IsLeaf(XelRBNode * NodePtr) {
    return !NodePtr->LeftNodePtr && !NodePtr->RightNodePtr;
}
static inline bool XRBN_IsRed(XelRBNode * NodePtr) {
    return NodePtr->RedFlag;
}
static inline bool XRBN_IsGenericRed(XelRBNode * NodePtr) {
    return NodePtr && (NodePtr->RedFlag);
}
static inline bool XRBN_IsBlack(XelRBNode * NodePtr) {
    return !XRBN_IsRed(NodePtr);
}
static inline bool XRBN_IsGenericBlack(XelRBNode * NodePtr) {
    return !NodePtr || !XRBN_IsRed(NodePtr);
}
static inline void XRBN_MarkRed(XelRBNode * NodePtr) {
    NodePtr->RedFlag = true;
}
static inline void XRBN_MarkBlack(XelRBNode * NodePtr) {
    NodePtr->RedFlag = false;
}

static inline void* XRBN_Cast(XelRBNode* NodePtr, size_t NodeMemberOffset) {
    if (!NodePtr) {
        return NULL;
    }
    return (void*)((unsigned char*)NodePtr - NodeMemberOffset);
}

static inline XelRBNode * XRBN_LeftMost(XelRBNode * NodePtr) {
    // assert(NodePtr);
    while (NodePtr->LeftNodePtr) {
        NodePtr = NodePtr->LeftNodePtr;
    }
    return NodePtr;
}

static inline XelRBNode * XRBN_RightMost(XelRBNode * NodePtr) {
    // assert(NodePtr);
    while (NodePtr->RightNodePtr) {
        NodePtr = NodePtr->RightNodePtr;
    }
    return NodePtr;
}

static inline XelRBNode * XRBN_Prev(XelRBNode * NodePtr) {
    XelRBNode * ParentPtr;
    if (NodePtr->LeftNodePtr) {
        return XRBN_RightMost(NodePtr->LeftNodePtr);
    }
    while ((ParentPtr = NodePtr->ParentPtr) && (NodePtr == ParentPtr->LeftNodePtr)) {
        NodePtr = ParentPtr;
    }
    return ParentPtr;
}

static inline XelRBNode * XRBN_Next(XelRBNode * NodePtr) {
    XelRBNode * ParentPtr;
    if (NodePtr->RightNodePtr) {
        return XRBN_LeftMost(NodePtr->RightNodePtr);
    }
    while ((ParentPtr = NodePtr->ParentPtr) && (NodePtr == ParentPtr->RightNodePtr)) {
        NodePtr = ParentPtr;
    }
    return ParentPtr;
}

#define XRBN_ENTRY(_What, Type, Member) ((Type*)(XRBN_Cast((_What), offsetof(Type, Member))))

/* Tree */
typedef struct XelRBTree XelRBTree;
struct XelRBTree {
    XelRBNode * RootPtr;
};

typedef int XRBT_KeyCompare(XelRBTree * TreePtr, const void * KeyPtr, XelRBNode * NodePtr);

static inline void XRBT_Init(XelRBTree* TreePtr) {
    XelRBTree InitValue = { NULL };
    *TreePtr = InitValue;
}

static inline bool XRBT_IsEmpty(XelRBTree* TreePtr) {
    return !TreePtr->RootPtr;
}

static inline void* XRBT_Cast(XelRBTree* TreePtr, size_t NodeMemberOffset) {
    if (!TreePtr) {
        return NULL;
    }
    return (void*)((unsigned char*)TreePtr - NodeMemberOffset);
}
#define XRBT_ENTRY(_What, Type, Member) ((Type*)(XRBT_Cast((_What), offsetof(Type, Member))))

static inline XelRBNode * XRBT_First(XelRBTree * TreePtr)
{
    return TreePtr->RootPtr ? XRBN_LeftMost(TreePtr->RootPtr) : NULL;
}

static inline XelRBNode * XRBT_Last(XelRBTree * TreePtr)
{
    return TreePtr->RootPtr ? XRBN_RightMost(TreePtr->RootPtr) : NULL;
}

static inline XelRBNode *XRBT_Find(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr) {
    XelRBNode * CurrNodePtr = TreePtr->RootPtr;
    while (CurrNodePtr) {
        int CompareResult = (*CompFunc)(TreePtr, KeyPtr, CurrNodePtr);
        if (CompareResult < 0) {
            CurrNodePtr = CurrNodePtr->LeftNodePtr;
        }
        else if (CompareResult > 0) {
            CurrNodePtr = CurrNodePtr->RightNodePtr;
        }
        else {
            return CurrNodePtr;
        }
    }
    return NULL;
}

static inline XelRBInsertSlot XRBT_FindInsertSlot(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void *KeyPtr) {
    XelRBInsertSlot InsertNode = { NULL, NULL };
    XelRBNode ** CurrNodeRefPtr = &TreePtr->RootPtr;
    while (*CurrNodeRefPtr) {
        InsertNode.ParentPtr = *CurrNodeRefPtr;
        int CompareResult = (*CompFunc)(TreePtr, KeyPtr, *CurrNodeRefPtr);
        if (CompareResult < 0) {
            CurrNodeRefPtr = &(*CurrNodeRefPtr)->LeftNodePtr;
        }
        else if (CompareResult > 0) {
            CurrNodeRefPtr = &(*CurrNodeRefPtr)->RightNodePtr;
        }
        else {
            CurrNodeRefPtr = NULL;
            break;
        }
    }
    InsertNode.SubNodeRefPtr = CurrNodeRefPtr;
    return InsertNode;
}

/* if a FindInsertSlot result indicates replacement, return the to-bo replaced node */
static inline XelRBNode * XRBT_Original(XelRBInsertSlot InsertSlot) {
    if (InsertSlot.SubNodeRefPtr) {
        return NULL;
    }
    return InsertSlot.ParentPtr;
}

static inline void XRBT_Replace(XelRBTree * TreePtr, XelRBInsertSlot InsertSlot, XelRBNode * NodePtr)
{
    assert(!InsertSlot.SubNodeRefPtr);
    if (!InsertSlot.ParentPtr) { // root
        TreePtr->RootPtr = NodePtr;
        return;
    }

    XelRBNode * ReplaceNodePtr = InsertSlot.ParentPtr;
    if ((NodePtr->LeftNodePtr = ReplaceNodePtr->LeftNodePtr)) {
        NodePtr->LeftNodePtr->ParentPtr = NodePtr;
    }
    if ((NodePtr->RightNodePtr = ReplaceNodePtr->RightNodePtr)) {
        NodePtr->RightNodePtr->ParentPtr = NodePtr;
    }
    NodePtr->RedFlag = ReplaceNodePtr->RedFlag;

    if ((NodePtr->ParentPtr = ReplaceNodePtr->ParentPtr)) {
        XelRBNode * ParentNodePtr = NodePtr->ParentPtr;
        if (ParentNodePtr->LeftNodePtr == ReplaceNodePtr) {
            ParentNodePtr->LeftNodePtr = NodePtr;
        } else {
            ParentNodePtr->RightNodePtr = NodePtr;
        }
    } else { // root
        TreePtr->RootPtr = NodePtr;
    }
    XRBN_Init(ReplaceNodePtr);
    return;
}

#define XRBT_FOR_EACH(_iter, _tree) \
    for (XelRBNode *_iter = XRBT_First((_tree)); _iter; _iter = XRBN_Next(_iter))

#define XRBT_FOR_EACH_SAFE(_iter, _tree) \
    for (XelRBNode *_iter = XRBT_First((_tree)), *_safe = XRBN_Next(_iter); _iter; _iter = _safe, _safe = XRBN_Next(_iter))


X_API void         XRBT_Insert(XelRBTree * TreePtr, XelRBInsertSlot InsertSlot, XelRBNode * NodePtr);
X_API XelRBNode *  XRBT_InsertOrAssign(XelRBTree * TreePtr, XelRBNode * NodePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr);
X_API void         XRBT_Remove(XelRBTree * TreePtr, XelRBNode * NodePtr);
X_API bool         XRBT_Check(XelRBTree * TreePtr);

#ifdef __cplusplus
}
#endif
