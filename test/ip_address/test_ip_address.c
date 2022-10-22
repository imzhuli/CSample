#include <xel_ext/net/X_IpAddress.h>
#include <string.h>
#include <stdio.h>

#define Assert(x) do { if (!(x)) { printf("Assertion failed: %s\n", #x); exit(-1); } } while(false)

int main(int argc, char *argv[])
{
	XelIpStr IpStr = XEL_IP_STR_INIT;

	const char * Ipv4Str = "192.168.31.5";
	XelInAddr Addr;
	Assert(X_StrToIpv4(&Addr, Ipv4Str));
	Assert(X_Ipv4ToStr(&IpStr, &Addr));
	Assert(0 == strcmp(Ipv4Str, IpStr.CharSequence));
	printf("ip string back: %s\n", IpStr.CharSequence);

	const char * Ipv6Str = "0000:0000:0000:0000:0000:ffff:192.168.100.228";
	XelIn6Addr Addr6;
	Assert(X_StrToIpv6(&Addr6, Ipv6Str));
	Assert(X_Ipv6ToStr(&IpStr, &Addr6));
	printf("ip6 string original: %s\n", Ipv6Str);
	printf("ip6 string back: %s\n", IpStr.CharSequence);

	XelIn6Addr Addr6Check;
	Assert(X_StrToIpv6(&Addr6Check, IpStr.CharSequence));
	Assert(0 == memcmp(&Addr6, &Addr6Check, sizeof(XelIn6Addr)));

	return 0;
}
