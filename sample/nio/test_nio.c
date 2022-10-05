#include <xel/X_Base.h>
#include <xel/X_Chrono.h>
#include <xel_ext/net/X_IO.h>
#include <xel_ext/net/X_TcpConnection.h>

XelIoContext        IoContext;
XelTcpConnection    TcpConnection;

int main(int argc, char * argv[])
{
	X_RTA(XIC_Init(&IoContext));
	X_RTA(XTC_InitConnect(&IoContext, &TcpConnection, "14.215.177.38", 80));

	uint64_t StartTimer = X_GetTimestampMS();
	while(X_GetTimestampMS() - StartTimer < 1125000) {
		XIC_LoopOnce(&IoContext, 1000);
	}

	XTC_Clean(&TcpConnection);
	XIC_Clean(&IoContext);
	return 0;
}
