#include <xel/X_Base.h>
#include <xel/X_Chrono.h>
#include <xel_ext/net/X_IO.h>
#include <xel_ext/net/X_TcpConnection.h>

XelIoContext        IoContext;
XelTcpConnection    TcpConnection;

static const char * HttpRequest =
"GET / HTTP/1.1\r\n"
"Host: www.baidu.com\r\n"
"User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:105.0) Gecko/20100101 Firefox/105.0\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
"Accept-Language: en-US,en;q=0.5\r\n"
"Accept-Encoding: gzip, deflate, br\r\n"
"Connection: close\r\n"
"\r\n";

XelTcpConnectionListener Listener =
{
	NULL,
	NULL,
	NULL,
};

int main(int argc, char * argv[])
{
	X_RTA(XIC_Init(&IoContext));
	X_RTA(XTC_InitConnect(&IoContext, &TcpConnection, "14.215.177.38", 80, &Listener));

	XTC_PostData(&TcpConnection, HttpRequest, strlen(HttpRequest));

	uint64_t StartTimer = X_GetTimestampMS();
	while(X_GetTimestampMS() - StartTimer < 15000) {
		XIC_LoopOnce(&IoContext, 1000);
	}

	XTC_Clean(&TcpConnection);
	XIC_Clean(&IoContext);
	return 0;
}
