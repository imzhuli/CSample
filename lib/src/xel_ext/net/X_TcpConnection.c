#include <xel_ext/net/X_TcpConnection.h>
#include <xel_ext/net/X_IpAddress.h>
#include <xel/X_String.h>
#include <errno.h>

#if defined(X_SYSTEM_LINUX)
	#include <sys/types.h>
	#include <sys/socket.h>
#else
#endif

static void XTC_FlushData(XelTcpConnection * TcpConnectionPtr)
{
    assert(TcpConnectionPtr->_Status == XTCS_Connected);
    XelWriteBufferChain * ChainPtr = &TcpConnectionPtr->_WriteBufferChain;
	while(true) {
		XelWriteBuffer * BufferPtr = XWBC_Peek(ChainPtr);
		if (!BufferPtr) {
			return;
		}
		ssize_t WB = send(TcpConnectionPtr->_Socket, BufferPtr->Buffer, (send_len_t)BufferPtr->BufferDataSize, XelNoWriteSignal);
		X_DbgInfo("SendData: socket=%i, size=%zi", TcpConnectionPtr->_Socket, (size_t)WB);
		if (WB == BufferPtr->BufferDataSize) {
			XWBC_FreeFront(ChainPtr);
			continue;
		}
		if (WB < 0) {
			X_DbgInfo("SendDataError: %s", strerror(errno));
			if (errno != EAGAIN) {
				XTC_Close(TcpConnectionPtr);
				return;
			}
			XIEB_MarkWriting(&TcpConnectionPtr->_IoEventBase);
			break;
		}
		if ((size_t)WB < BufferPtr->BufferDataSize) {
			BufferPtr->BufferDataSize -= WB;
			memmove(BufferPtr->Buffer, BufferPtr->Buffer + WB, BufferPtr->BufferDataSize);
			break;
		}
	}

}

static void XTC_EventCallback(XelIoEventBase * IoEventBasePtr, XelIoEventType IoEventType)
{
	XelTcpConnection * TcpConnectionPtr = X_Entry(IoEventBasePtr, XelTcpConnection, _IoEventBase);

#if defined (X_SYSTEM_LINUX)
	if (IoEventType == XIET_In) {
		X_DbgInfo("IoEventIn");
		while(true) {
			XelUByte Buffer[1024];
    		ssize_t Rb = recv(TcpConnectionPtr->_Socket, Buffer, (recv_len_t)sizeof(Buffer), 0);
			if (Rb > 0) {
				XelString Hex = XS_HexShow(Buffer, Rb, true);
				X_DbgInfo("RecvData: size=%zi, Hex=\n%s\n", Rb, XS_GetData(Hex));
				XS_Delete(Hex);
			}
			if (Rb == 0) {
				X_DbgInfo("PeerClose");
				XTC_Close(TcpConnectionPtr);
				return;
			}
			if (Rb < 0) {
				if (errno == EAGAIN) {
					return;
				}
				XTC_Close(TcpConnectionPtr);
				return;
			}
		}
		return;
	}
	if (IoEventType == XIET_Out) {
		if (TcpConnectionPtr->_Status == XTCS_Connecting) {
			TcpConnectionPtr->_Status = XTCS_Connected;
			TcpConnectionPtr->_EventListener.OnConnected(TcpConnectionPtr, TcpConnectionPtr->_EventListener.ContextPtr);
			if (!XTC_IsConnected(TcpConnectionPtr)) {
				X_DbgInfo("ConnectionClosed during connected event callback");
				return;
			}
		}
		XTC_FlushData(TcpConnectionPtr);
		return;
	}
	if (IoEventType == XIET_Err) {
		close(TcpConnectionPtr->_Socket);
		TcpConnectionPtr->_Socket = XelInvalidSocket;
		TcpConnectionPtr->_Status = XTCS_Closed;
		X_DbgError("IoEvent");
		return;
	}
#endif
}

static void TrivialOnConnectedCallback(XelTcpConnection * TcpConnectionPtr, void * ContextPtr)
{
	// do nothing
}

static void XTC_SetEventListener(XelTcpConnection * TcpConnectionPtr, const XelTcpConnectionListener * ListenerPtr)
{
	TcpConnectionPtr->_EventListener = *ListenerPtr;
	if (!TcpConnectionPtr->_EventListener.OnData) {
		// TODO
	}
	if (!TcpConnectionPtr->_EventListener.OnConnected) {
		TcpConnectionPtr->_EventListener.OnConnected = &TrivialOnConnectedCallback;
	}
}

bool XTC_InitConnect(XelIoContext * IoContextPtr, XelTcpConnection * TcpConnectionPtr, const char * IpString, uint16_t Port, const XelTcpConnectionListener * ListenerPtr)
{
	TcpConnectionPtr->_Socket = XelInvalidSocket;
	TcpConnectionPtr->_Status = XTCS_Closed;

	if (!XWBC_Init(&TcpConnectionPtr->_WriteBufferChain, NULL)) {
		X_FatalAbort("Failed to write buffer chain");
		return false;
	}

	XelInAddr  RemoteSinAddr = { 0 };
	XelIn6Addr RemoteSin6Addr = { 0 };
	XelIoEventBase * EventBasePtr = &TcpConnectionPtr->_IoEventBase;
	if (!XIEB_Init(EventBasePtr)) {
		X_FatalAbort("Failed to init event base");
		return false;
	}

#if defined (X_SYSTEM_LINUX)
	if (X_StrToIpv4(&RemoteSinAddr, IpString)) {
		int Fd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == Fd) {
			XIEB_Clean(&TcpConnectionPtr->_IoEventBase);
			XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
			return false;
		}
		XS_SetNonBlocking(Fd);
		struct sockaddr_in Sockaddr = { 0 };
		Sockaddr.sin_family = AF_INET;
		Sockaddr.sin_addr = RemoteSinAddr;
		Sockaddr.sin_port = htons(Port);
		if (-1 == connect(Fd, (struct sockaddr*)&Sockaddr, (socklen_t)sizeof(Sockaddr))) {
			if (errno != EINPROGRESS) {
				close(TcpConnectionPtr->_Socket);
				TcpConnectionPtr->_Socket = XelInvalidSocket;
				TcpConnectionPtr->_Status = XTCS_Closed;
				XIEB_Clean(EventBasePtr);
				XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
				return false;
			}
			X_DbgInfo("Connection to %s is inprogress, IoHandle=%i", IpString, Fd);
			TcpConnectionPtr->_Socket = Fd;
			TcpConnectionPtr->_Status = XTCS_Connecting;
			XIEB_MarkWriting(EventBasePtr);

			XelIoHandle IoHandle = { .IoType = XIT_Socket, .FileDescriptor = Fd };
			if (!XIEB_Bind(IoContextPtr, EventBasePtr, IoHandle, XTC_EventCallback)) {
				X_DbgError("Failed to bind IoEventBase to IoContext");
				close(TcpConnectionPtr->_Socket);
				TcpConnectionPtr->_Socket = XelInvalidSocket;
				TcpConnectionPtr->_Status = XTCS_Closed;
				XIEB_Clean(EventBasePtr);
				XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
				return false;
			}
		}
		else {
			TcpConnectionPtr->_Socket = Fd;
			TcpConnectionPtr->_Status = XTCS_Connected;
			X_DbgInfo("Connection to %s is connected", IpString);
		}
		XIEB_ResumeReading(EventBasePtr);
	}
	else if (X_StrToIpv6(&RemoteSin6Addr, IpString)) {
		XIEB_Clean(EventBasePtr);
		XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
		X_FatalAbort("XTC_InitConnect ipv6 not supported");
		return false;
	}
	else {
		XIEB_Clean(&TcpConnectionPtr->_IoEventBase);
		XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
		X_FatalAbort("XTC_InitConnect invalid ip type");
		return false;
	}

#else
	X_FatalAbort("XTC_InitConnect not implemented");
#endif

	XTC_SetEventListener(TcpConnectionPtr, ListenerPtr);
	return true;
}

void XTC_Close(XelTcpConnection * TcpConnectionPtr)
{
	if (TcpConnectionPtr->_Status == XTCS_Closed) {
		return;
	}
	XIEB_Unbind(&TcpConnectionPtr->_IoEventBase);
	close(TcpConnectionPtr->_Socket);
	TcpConnectionPtr->_Socket = XelInvalidSocket;
	TcpConnectionPtr->_Status = XTCS_Closed;
}

void XTC_Clean(XelTcpConnection * TcpConnectionPtr)
{
	if (TcpConnectionPtr->_Status != XTCS_Closed) {
		XTC_Close(TcpConnectionPtr);
	}
	XIEB_Clean(&TcpConnectionPtr->_IoEventBase);
	XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
}

size_t XTC_PostData(XelTcpConnection * TcpConnectionPtr, const void * DataPtr_, size_t Size)
{
	assert(TcpConnectionPtr->_Status != XTCS_Closed);
	XelUByte * DataPtr = (XelUByte *)DataPtr_;
	XelWriteBufferChain * WriteBufferChainPtr = &TcpConnectionPtr->_WriteBufferChain;
	if (XIEB_GetWritingMark(&TcpConnectionPtr->_IoEventBase) || !XWBC_IsEmpty(WriteBufferChainPtr)) {
		return XWBC_PushBack(WriteBufferChainPtr, DataPtr, Size);
	}
	// send
#if defined (X_SYSTEM_LINUX)
	ssize_t WB = send(TcpConnectionPtr->_Socket, DataPtr, Size, XelNoWriteSignal);
	X_DbgInfo("SendData: %zi", (size_t)WB);
	if (WB < 0) {
        if (errno != EAGAIN) {
			XIEB_Unbind(&TcpConnectionPtr->_IoEventBase);
			close(TcpConnectionPtr->_Socket);
			TcpConnectionPtr->_Socket = XelInvalidSocket;
			TcpConnectionPtr->_Status = XTCS_Closed;
			return 0;
		}
	}
	else {
		DataPtr += WB;
		Size -= WB;
	}
	if (Size) {
		size_t Total = (size_t)WB + XWBC_PushBack(WriteBufferChainPtr, DataPtr, Size);
		return Total;
	}
	return WB;
#endif

	X_FatalAbort("XTC_PostData not implemented");
	return 0;
}
