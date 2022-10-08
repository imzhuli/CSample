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
typedef struct XelTcpConnectionListener XelTcpConnectionListener;

struct XelTcpConnectionListener
{
	void * ContextPtr;
	void (*OnData)(XelTcpConnection * ConnectinPtr, void * ContextPtr, XelUByte * DataPtr, size_t DataSize);
	void (*OnConnected)(XelTcpConnection * ConnectionPtr, void * ContextPtr);
};

struct XelTcpConnection
{
	XelIoEventBase            _IoEventBase;
	XelSocket                 _Socket;
	XelTcpConnectionStatus    _Status;

	XelUByte                  _ReadBuffer[XelPacketMaxSize]; // at least of sizeof a full packet
	XelWriteBufferChain       _WriteBufferChain;
	XelTcpConnectionListener  _EventListener;
};
X_STATIC_INLINE bool XTC_IsConnected(const XelTcpConnection * TcpConnectionPtr) { return TcpConnectionPtr->_Status == XTCS_Connected; }

X_API bool    XTC_InitConnect(XelIoContext * IoContext, XelTcpConnection * TcpConnectionPtr, const char * IpString, uint16_t port, const XelTcpConnectionListener * ListenerPtr);
X_API void    XTC_Close(XelTcpConnection * TcpConnectionPtr);
X_API void    XTC_Clean(XelTcpConnection * TcpConnectionPtr);
X_API size_t  XTC_PostData(XelTcpConnection * TcpConntionPtr, const void * DataPtr, size_t Size);


X_CNAME_END
