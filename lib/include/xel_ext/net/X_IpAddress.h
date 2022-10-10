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
#define XEL_IP_STR_INIT {{0}}

typedef struct in_addr  XelInAddr;
#define XEL_IN_ADDR_INIT {0}
X_API XelInAddr XelInAddrZero;

typedef struct in6_addr XelIn6Addr;
#define XEL_IN6_ADDR_INIT {{{0}}}
X_API XelIn6Addr XelIn6AddrZero;

X_API bool X_Ipv4ToStr(XelIpStr * OutputPtr, const XelInAddr  * SinAddrPtr);
X_API bool X_Ipv6ToStr(XelIpStr * OutputPtr, const XelIn6Addr * Sin6AddrPtr);

X_API bool X_StrToIpv4(XelInAddr  * OutputPtr, const char * Str);
X_API bool X_StrToIpv6(XelIn6Addr * OutputPtr, const char * Str);

X_CNAME_END
