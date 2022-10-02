#include <xel/X_IndexedStorage.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define TestSize 512

static void TestIdPool()
{
    XelIndexIdPool IdPool;
    XelIndexIdPool * PoolPtr = &IdPool;
    XIP_Init(PoolPtr, TestSize);
    uint64_t FailKey = XelInvalidIndexId;
    uint64_t Keys[TestSize];
    memset(Keys, 0, sizeof(Keys));

    if (XIP_Check(PoolPtr, (XelIndexId)-1)) {
        printf("TestIdPool Fatal: Attack key allowd\n");
        goto Error;
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        Keys[i] = XIP_Acquire(PoolPtr);
        if (Keys[i] == XelInvalidIndexId) {
            printf("TestIdPool Failed to acquire index, IndexId=%zi\n", i);
            goto Error;
        }
    }
    FailKey = XIP_Acquire(PoolPtr);
    if (FailKey != XelInvalidIndexId) {
        printf("TestIdPool Failed to fail on acquire over TestSize\n");
        goto Error;
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        if (!XIP_CheckAndRelease(PoolPtr, Keys[i])) {
            printf("TestIdPool Failed to check and release valid key, IndexId=%zi\n", i);
            goto Error;
        }
        Keys[i] = 0;
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        Keys[i] = XIP_Acquire(PoolPtr);
        if (Keys[i] == XelInvalidIndexId) {
            printf("TestIdPool Second phase acquire Failed, IndexId=%zi\n", i);
            goto Error;
        }
    }

    FailKey = XIP_Acquire(PoolPtr);
    if (FailKey != XelInvalidIndexId) {
        printf("TestIdPool Failed to fail on acquire over TestSize\n");
        goto Error;
    }

    XIP_Clean(PoolPtr);
    return;

Error:
    XIP_Clean(PoolPtr);
    exit(-1);
}

static void TestIdStorage()
{
    XelIndexedStoragePool Pool;
    XelIndexedStoragePool * PoolPtr = &Pool;
    uint64_t FailKey = XelInvalidIndexId;
    uint64_t Keys[TestSize];
    memset(Keys, 0, sizeof(Keys));    

    XISP_Init(PoolPtr, TestSize);
    
    if (XISP_Check(PoolPtr, (XelIndexId)-1)) {
        printf("TestIdStorage Fatal: Attack key allowd\n");
        goto Error;
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        XelVariable X = { .U32 = (uint32_t)i };
        Keys[i] = XISP_Acquire(PoolPtr, X);
        if (Keys[i] == XelInvalidIndexId) {
            printf("TestIdStorage Failed to acquire index, IndexId=%zi\n", i);
            goto Error;
        }
    }

    FailKey = XISP_Acquire(PoolPtr, (XelVariable){ .U32 = 0xFFFFFFFF });
    if (FailKey != XelInvalidIndexId) {
        printf("TestIdStorage Failed to fail on acquire over TestSize\n");
        goto Error;
    }    

    for (size_t i = 0 ; i < TestSize; ++i) {
        if (Keys[i] == XelInvalidIndexId) {
            printf("TestIdStorage Failed to acquire index, IndexId=%zi\n", i);
            goto Error;
        }
        XelOptionalVariable Opt = XISP_CheckAndGet(PoolPtr, Keys[i]);
        if (!Opt.HasValue || Opt.Value.U32 != (uint32_t)i) {
            printf("TestIdStorage faild to get oringally stored value\n");
            goto Error;
        }
        XelVariable * ValuePtr = XISP_CheckAndGetRef(PoolPtr, Keys[i]);
        if (!ValuePtr || ValuePtr->U32 != (uint32_t)i) {            
            printf("TestIdStorage faild to get valid reference to oringally stored value\n");
            goto Error;
        }
        if (!XISP_CheckAndRelease(PoolPtr, Keys[i])) {   
            printf("TestIdStorage faild to check and release, index=%" PRIu32 ", id=%" PRIu64 ", \n", (uint32_t)i, Keys[i]);
            goto Error;
        }
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        XelVariable X = { .U32 = (uint32_t)i };
        Keys[i] = XISP_Acquire(PoolPtr, X);
        if (Keys[i] == XelInvalidIndexId) {
            printf("TestIdStorage second phase failed to acquire index, IndexId=%zi\n", i);
            goto Error;
        }
    }

    FailKey = XISP_Acquire(PoolPtr, (XelVariable){ .U32 = 0xFFFFFFFF });
    if (FailKey != XelInvalidIndexId) {
        printf("TestIdStorage second phase failed to fail on acquire over TestSize\n");
        goto Error;
    }

    XISP_Clean(PoolPtr);
    return;

Error:
    XISP_Clean(PoolPtr);
    exit(-1);
}

int main(int argc, char * argv[])
{
    TestIdPool();
    TestIdStorage();
    return 0;
}
