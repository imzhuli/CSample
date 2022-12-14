#include <xel_ext/net/X_IpAddress.h>

XelInAddr  XelInAddrZero  = XEL_IN_ADDR_INIT;
XelIn6Addr XelIn6AddrZero = XEL_IN6_ADDR_INIT;

union XelIpv4Punning
{
	uint32_t S_Addr;
	XelUByte Raw[4];
};
typedef union XelIpv4Punning XelIpv4Punning;

bool X_Ipv4ToStr(XelIpStr * OutputPtr, const XelInAddr * SinAddrPtr)
{
    OutputPtr->CharSequence[0] = '\0';
    return inet_ntop(AF_INET, SinAddrPtr, OutputPtr->CharSequence, sizeof(OutputPtr->CharSequence));
}

bool X_Ipv6ToStr(XelIpStr * OutputPtr, const XelIn6Addr * Sin6AddrPtr)
{
    OutputPtr->CharSequence[0] = '\0';
    return inet_ntop(AF_INET6, Sin6AddrPtr, OutputPtr->CharSequence, sizeof(OutputPtr->CharSequence));
}

bool X_StrToIpv4(XelInAddr * OutputPtr, const char * Str)
{
    return inet_pton(AF_INET, Str, OutputPtr);
}

bool X_StrToIpv6(XelIn6Addr * OutputPtr, const char * Str)
{
    return inet_pton(AF_INET6, Str, OutputPtr);
}
