#include <xel_ext/net/X_IpAddress.h>

union XelIpv4Punning
{
	uint32_t S_Addr;
	XelUByte Raw[4];
};
typedef union XelIpv4Punning XelIpv4Punning;

XelIpStr X_Ipv4ToStr(const uint32_t S_Addr)
{
    XelIpStr Ret;
    XelIpv4Punning Punning;
    Punning.S_Addr = S_Addr;
    snprintf(Ret.CharSequence, sizeof(Ret.CharSequence), "%d.%d.%d.%d",
        (int)Punning.Raw[0],
        (int)Punning.Raw[1],
        (int)Punning.Raw[2],
        (int)Punning.Raw[3]);
    Ret.CharSequence[15] = '\0';
    return Ret;
}