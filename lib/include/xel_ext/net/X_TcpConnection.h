#pragma once
#include <xel/X_Base.h>
#include "./X_IO.h"

X_CNAME_BEGIN

enum XelTcpConnectionStatus
{
	XTCS_Idle,
	XTCS_Connecting,
	XTCS_Connected,
	XTCS_Closed,
};
typedef enum XelTcpConnectionStatus XelTcpConnectionStatus;

typedef struct XelTcpConnection XelTcpConnection;
struct XelTcpConnection
{
	XelIoEventBase          IoEventBase;
	XelTcpConnectionStatus  Status;
};

X_API bool XTC_InitConnect(XelIoContext * IoContext, XelTcpConnection * TcpConnectionPtr, const char * IpString, uint16_t port);
X_API void XTC_Clean(XelTcpConnection * TcpConnectionPtr);



X_CNAME_END
