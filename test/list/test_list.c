#include <xel/X_List.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_NODE_COUNT 1024

typedef struct xNode
{
    XelListNode ListNode;
    size_t      Value;
} xNode;

void InitNode(xNode * Node)
{
    XLN_Init(&Node->ListNode);
};

int main(int argc, char *argv[])
{
    size_t Counter = 0;
    XelList List;
    XL_Init(&List);

    xNode * NodePool = (xNode *)malloc(sizeof(xNode) * TEST_NODE_COUNT);

    for (size_t i = 0 ; i < TEST_NODE_COUNT; ++i) {
        xNode * Current = &NodePool[i];
        InitNode(Current);
        Current->Value = i;

        XL_AddHead(&List, &Current->ListNode);
    }
    
    for (XelListForwardIterator It = XL_Begin(&List); !XLFI_IsEqual(It, XL_End(&List)); It = XLFI_Next(It)) 
    {
        xNode * EntryPtr = X_Entry(XLFI_Get(It), xNode, ListNode);

        if (Counter + EntryPtr->Value != TEST_NODE_COUNT - 1) {
            printf("Failed to add node to head (disordered)\n");
            exit(-1);
        }

        XLN_Detach(XLFI_Get(It));
        ++Counter;
    }
    
    for (size_t i = 0 ; i < TEST_NODE_COUNT; ++i) {
        XL_GrabTail(&List, &NodePool[i].ListNode);
    }
    Counter = 0;
    for (XelListForwardIterator It = XL_Begin(&List); !XLFI_IsEqual(It, XL_End(&List)); It = XLFI_Next(It)) 
    {
        xNode * EntryPtr = X_Entry(XLFI_Get(It), xNode, ListNode);

        if (Counter != EntryPtr->Value) {
            printf("Failed to grab node to tail (disordered)\n");
            exit(-1);
        }

        XLN_Detach(XLFI_Get(It));
        ++Counter;
    }

    for (size_t i = 0 ; i < TEST_NODE_COUNT; ++i) {
        if (XLN_IsLinked(&NodePool[i].ListNode)) {
            printf("Detach cleaning bug: NodePool[%zi] is still linked\n", i);
            exit(-1);
        }
    }

    if (!XL_IsEmpty(&List)) {
        printf("List should be empty, since every node is detached\n");
        exit(-1);
    }

    free(NodePool);
    return 0;
}