#pragma once
#include <xel/X_Base.h>
#include "./X_IO.h"
#include "./X_Packet.h"
#include "./X_PacketBuffer.h"

X_CNAME_BEGIN

enum XelTcpConnectionStatus
{
	XTCS_Closed,
	XTCS_Connecting,
	XTCS_Connected,
};
typedef enum XelTcpConnectionStatus XelTcpConnectionStatus;

typedef struct XelTcpConnection XelTcpConnection;
struct XelTcpConnection
{
	XelIoEventBase          IoEventBase;
	XelSocket               Socket;
	XelTcpConnectionStatus  Status;

	XelUByte                ReadBuffer[XelPacketMaxSize]; // at least of sizeof a full packet
	XelWriteBufferChain     WriteBufferChain;
};

X_API bool    XTC_InitConnect(XelIoContext * IoContext, XelTcpConnection * TcpConnectionPtr, const char * IpString, uint16_t port);
X_API void    XTC_Close(XelTcpConnection * TcpConnectionPtr);
X_API void    XTC_Clean(XelTcpConnection * TcpConnectionPtr);
X_API size_t  XTC_PostData(XelTcpConnection * TcpConntionPtr, const void * DataPtr, size_t Size);


X_CNAME_END
