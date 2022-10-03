#include <xel_ext/net/X_IpAddress.h>
#include <string.h>
#include <stdio.h>

#define Assert(x) do { if (!(x)) { printf("Assertion failed: %s\n", #x); exit(-1); } } while(false)

int main(int argc, char *argv[])
{
	const char * Ipv4Str = "192.168.31.5";
	XelInAddr Addr = { 0 };
	Assert(X_StrToIpv4(&Addr, Ipv4Str));
	XelIpStr IpStr = { 0 };
	Assert(X_Ipv4ToStr(&IpStr, &Addr));
	Assert(0 == strcmp(Ipv4Str, IpStr.CharSequence));

	printf("ip string back: %s\n", IpStr.CharSequence);

	return 0;
}
