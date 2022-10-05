#include <xel_ext/net/X_TcpConnection.h>
#include <xel_ext/net/X_IpAddress.h>
#include <errno.h>

#if defined(X_SYSTEM_LINUX)
	#include <sys/types.h>
	#include <sys/socket.h>
#else
#endif

static void XTC_EventCallback(XelIoEventBase * IoEventBasePtr, XelIoEventType IoEventType, XelIoHandle IoHandle)
{
	XelTcpConnection * ConnectionPtr = X_Entry(IoEventBasePtr, XelTcpConnection, IoEventBase);

#if defined (X_SYSTEM_LINUX)
	if (IoEventType == XIET_In) {
		X_DbgInfo("IoEventIn");
		return;
	}
	if (IoEventType == XIET_Out) {
		if (ConnectionPtr->Status == XTCS_Connecting) {
			ConnectionPtr->Status = XTCS_Connected;
			X_DbgInfo("IoEventConnected");
		}
		else {
			X_DbgInfo("IoEvenOut");
		}
		return;
	}
	if (IoEventType == XIET_Err) {
		ConnectionPtr->Status = XTCS_Closed;
		close(IoHandle.FileDescriptor);
		X_DbgError("IoEvent");
		return;
	}
#endif
}

bool XTC_InitConnect(XelIoContext * IoContextPtr, XelTcpConnection * TcpConnectionPtr, const char * IpString, uint16_t Port)
{
	XelInAddr  RemoteSinAddr = { 0 };
	XelIn6Addr RemoteSin6Addr = { 0 };
	XelIoEventBase * EventBasePtr = &TcpConnectionPtr->IoEventBase;
	if (!XIEB_Init(EventBasePtr)) {
		X_FatalAbort("Failed to init event base");
		return false;
	}

#if defined (X_SYSTEM_LINUX)
	if (X_StrToIpv4(&RemoteSinAddr, IpString)) {
		int Fd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == Fd) {
			XIEB_Clean(&TcpConnectionPtr->IoEventBase);
			return false;
		}
		XS_SetNonBlocking(Fd);
		struct sockaddr_in Sockaddr = { 0 };
		Sockaddr.sin_family = AF_INET;
		Sockaddr.sin_addr = RemoteSinAddr;
		Sockaddr.sin_port = htons(Port);
		if (-1 == connect(Fd, (struct sockaddr*)&Sockaddr, (socklen_t)sizeof(Sockaddr))) {
			if (errno != EINPROGRESS) {
				XIEB_Clean(EventBasePtr);
				return false;
			}
			X_DbgInfo("Connection to %s is inprogress, IoHandle=%i", IpString, Fd);
			TcpConnectionPtr->Status = XTCS_Connecting;
			XIEB_MarkWriting(EventBasePtr);

			XelIoHandle IoHandle = { .IoType = XIT_Socket, .FileDescriptor = Fd };
			if (!XIEB_Bind(IoContextPtr, EventBasePtr, IoHandle, XTC_EventCallback)) {
				X_DbgError("Failed to bind IoEventBase to IoContext");
				XIEB_Clean(EventBasePtr);
				return false;
			}
		}
		else {
			TcpConnectionPtr->Status = XTCS_Connected;
			X_DbgInfo("Connection to %s is connected", IpString);
		}
		XIEB_ResumeReading(EventBasePtr);
	}
	else if (X_StrToIpv6(&RemoteSin6Addr, IpString)) {
		XIEB_Clean(EventBasePtr);
		X_FatalAbort("XTC_InitConnect ipv6 not supported");
		return false;
	}
	else {
		XIEB_Clean(&TcpConnectionPtr->IoEventBase);
		X_FatalAbort("XTC_InitConnect invalid ip type");
		return false;
	}

#else
	X_FatalAbort("XTC_InitConnect not implemented");
#endif

	return true;
}

void XTC_Clean(XelTcpConnection * TcpConnectionPtr)
{
	XIEB_Clean(&TcpConnectionPtr->IoEventBase);

}
