#pragma once
#include "./X_IO.h"
#include <stdio.h>
#include <string.h>

X_CNAME_BEGIN

typedef struct XelIpStr {
	// standard ipv6:
	// len:39:   0000:0000:0000:0000:0000:0000:0000:0000
	// mapped ipv4:
	// len:45    0000:0000:0000:0000:0000:ffff:192.168.100.228
	// INET6_ADDRSTRLEN = 46: 45 + trailing null
	char CharSequence[46];
} XelIpStr;

typedef struct in_addr  XelInAddr;
typedef struct in6_addr XelIn6Addr;

X_API bool X_Ipv4ToStr(XelIpStr * OutputPtr, XelInAddr  * SinAddrPtr);
X_API bool X_Ipv6ToStr(XelIpStr * OutputPtr, XelIn6Addr * Sin6AddrPtr);

X_API bool X_StrToIpv4(XelInAddr  * OutputPtr, const char * Str);
X_API bool X_StrToIpv6(XelIn6Addr * OutputPtr, const char * Str);

X_CNAME_END
