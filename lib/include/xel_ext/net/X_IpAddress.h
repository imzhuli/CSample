#pragma once
#include "./X_IO.h"
#include <stdio.h>
#include <string.h>

X_CNAME_BEGIN

typedef struct XelIpv4Str {
    char CharSequence[16];
} XelIpStr;

X_API XelIpStr X_Ipv4ToStr(const uint32_t S_Addr);
X_API XelIpStr X_Ipv6ToStr(const uint32_t S_Addr);

X_CNAME_END
