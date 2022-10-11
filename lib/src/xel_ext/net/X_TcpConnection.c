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
				TcpConnectionPtr->_EventListener.OnErrorClosed(TcpConnectionPtr, TcpConnectionPtr->_EventListener.ContextPtr);
				return;
			}
			XIEB_MarkWriting(&TcpConnectionPtr->_IoEventBase);
			break;
		}
		if ((size_t)WB < BufferPtr->BufferDataSize) {
			BufferPtr->BufferDataSize -= WB;
			memmove(BufferPtr->Buffer, BufferPtr->Buffer + WB, BufferPtr->BufferDataSize);
			TcpConnectionPtr->_WriteBufferDataSize -= WB;
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
			XelUByte * BufferPtr = TcpConnectionPtr->_ReadBuffer + TcpConnectionPtr->_ReadDataSize;
			size_t SpaceSize = sizeof(TcpConnectionPtr->_ReadBuffer) - TcpConnectionPtr->_ReadDataSize;
			assert(SpaceSize && "No space for reading, this must be a bug, since it's required to consume at least 1 byte when read buffer is full");

			const ssize_t Rb = recv(TcpConnectionPtr->_Socket, BufferPtr, (recv_len_t)SpaceSize, 0);
			if (Rb > 0) {
				TcpConnectionPtr->_ReadDataSize += Rb;
				size_t ConsumedSize = TcpConnectionPtr->_EventListener.OnData(
					TcpConnectionPtr,
					TcpConnectionPtr->_EventListener.ContextPtr,
					TcpConnectionPtr->_ReadBuffer, TcpConnectionPtr->_ReadDataSize);
				if (ConsumedSize) {
					if (ConsumedSize == TcpConnectionPtr->_ReadDataSize) {
						TcpConnectionPtr->_ReadDataSize = 0;
					}
					else {
						size_t RemainSize = TcpConnectionPtr->_ReadDataSize - ConsumedSize;
						memmove(TcpConnectionPtr->_ReadBuffer, TcpConnectionPtr->_ReadBuffer + ConsumedSize, RemainSize);
						TcpConnectionPtr->_ReadDataSize = RemainSize;
					}
				}
			}
			else if (Rb == 0) {
				X_DbgInfo("PeerClose");
				XTC_Close(TcpConnectionPtr);
				TcpConnectionPtr->_EventListener.OnPeerClosed(TcpConnectionPtr, TcpConnectionPtr->_EventListener.ContextPtr);
				return;
			}
			else { // Rb < 0
				if (errno == EAGAIN) {
					return;
				}
				XTC_Close(TcpConnectionPtr);
				TcpConnectionPtr->_EventListener.OnErrorClosed(TcpConnectionPtr, TcpConnectionPtr->_EventListener.ContextPtr);
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
		XTC_Close(TcpConnectionPtr);
		TcpConnectionPtr->_EventListener.OnErrorClosed(TcpConnectionPtr, TcpConnectionPtr->_EventListener.ContextPtr);
		return;
	}
#endif
}

static size_t DebugPrintOnDataCallback(XelTcpConnection * ConnectinPtr, void * ContextPtr, XelUByte * DataPtr, size_t DataSize)
{
	XelString Hex = XS_HexShow(DataPtr, DataSize, true);
	X_DbgInfo("RecvData: size=%zi, Hex=\n%s\n", DataSize, XS_GetData(Hex));
	XS_Delete(Hex);
	return DataSize;
}

static void TrivialOnConnectedCallback(XelTcpConnection * TcpConnectionPtr, void * ContextPtr)
{
	// do nothing
}

static void TrivialOnPeerClosedCallback(XelTcpConnection * TcpConnectionPtr, void * ContextPtr)
{
	// do nothing
}

static void XTC_SetEventListener(XelTcpConnection * TcpConnectionPtr, const XelTcpConnectionListener * ListenerPtr)
{
	TcpConnectionPtr->_EventListener = *ListenerPtr;
	if (!TcpConnectionPtr->_EventListener.OnData) {
		TcpConnectionPtr->_EventListener.OnData = &DebugPrintOnDataCallback;
	}
	if (!TcpConnectionPtr->_EventListener.OnConnected) {
		TcpConnectionPtr->_EventListener.OnConnected = &TrivialOnConnectedCallback;
	}
	if (!TcpConnectionPtr->_EventListener.OnPeerClosed) {
		TcpConnectionPtr->_EventListener.OnPeerClosed = &TrivialOnPeerClosedCallback;
	}
	if (!TcpConnectionPtr->_EventListener.OnErrorClosed) {
		TcpConnectionPtr->_EventListener.OnErrorClosed = TcpConnectionPtr->_EventListener.OnPeerClosed;
	}
}

bool XTC_InitConnect(XelIoContext * IoContextPtr, XelTcpConnection * TcpConnectionPtr, const char * IpString, uint16_t Port, const XelTcpConnectionListener * ListenerPtr)
{
	TcpConnectionPtr->_Socket = XelInvalidSocket;
	TcpConnectionPtr->_Status = XTCS_Closed;
	XIUE_Init(&TcpConnectionPtr->_ExtraIntenalEventNode);

	TcpConnectionPtr->_ReadDataSize = 0;
	TcpConnectionPtr->_WriteBufferDataSize = 0;
	if (!XWBC_Init(&TcpConnectionPtr->_WriteBufferChain, NULL)) {
		return false;
	}

	XelInAddr  RemoteSinAddr = XEL_IN_ADDR_INIT;
	XelIn6Addr RemoteSin6Addr = XEL_IN6_ADDR_INIT;
	XelIoEventBase * EventBasePtr = &TcpConnectionPtr->_IoEventBase;
	if (!XIEB_Init(EventBasePtr)) {
		XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
		XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
		return false;
	}

#if defined (X_SYSTEM_LINUX)
	if (X_StrToIpv4(&RemoteSinAddr, IpString)) {
		int Fd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 == Fd) {
			XIEB_Clean(&TcpConnectionPtr->_IoEventBase);
			XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
			XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
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
				XIEB_Clean(EventBasePtr);
				XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
				XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
				return false;
			}
			TcpConnectionPtr->_Socket = Fd;
			TcpConnectionPtr->_Status = XTCS_Connecting;
			X_DbgInfo("Connection to %s is inprogress, IoHandle=%i", IpString, Fd);
			XIEB_MarkWriting(EventBasePtr);
		}
		else {
			TcpConnectionPtr->_Socket = Fd;
			TcpConnectionPtr->_Status = XTCS_Connected;
			X_DbgInfo("Connection to %s is connected", IpString);
		}
		XelIoHandle IoHandle = { .IoType = XIT_Socket, .FileDescriptor = Fd };
		if (!XIEB_Bind(IoContextPtr, EventBasePtr, IoHandle, XTC_EventCallback)) {
			X_DbgError("Failed to bind IoEventBase to IoContext");
			close(TcpConnectionPtr->_Socket);
			TcpConnectionPtr->_Socket = XelInvalidSocket;
			TcpConnectionPtr->_Status = XTCS_Closed;
			XIEB_Clean(EventBasePtr);
			XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
			XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
			return false;
		}
		XIEB_ResumeReading(EventBasePtr);
	}
	else if (X_StrToIpv6(&RemoteSin6Addr, IpString)) {
		XIEB_Clean(EventBasePtr);
		XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
		XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
		X_FatalAbort("XTC_InitConnect ipv6 not supported");
		return false;
	}
	else {
		XIEB_Clean(&TcpConnectionPtr->_IoEventBase);
		XWBC_Clean(&TcpConnectionPtr->_WriteBufferChain);
		XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
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
	XIUE_Detach(&TcpConnectionPtr->_ExtraIntenalEventNode);
	XelCloseSocket(TcpConnectionPtr->_Socket);
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
	XIUE_Clean(&TcpConnectionPtr->_ExtraIntenalEventNode);
}

size_t XTC_PostData(XelTcpConnection * TcpConnectionPtr, const void * DataPtr_, size_t Size)
{
	assert(TcpConnectionPtr->_Status != XTCS_Closed);
	XelUByte * DataPtr = (XelUByte *)DataPtr_;
	XelWriteBufferChain * WriteBufferChainPtr = &TcpConnectionPtr->_WriteBufferChain;
	if (XIEB_GetWritingMark(&TcpConnectionPtr->_IoEventBase) || !XWBC_IsEmpty(WriteBufferChainPtr)) {		
		size_t PushSize = XWBC_PushBack(WriteBufferChainPtr, DataPtr, Size);
		TcpConnectionPtr->_WriteBufferDataSize += PushSize;
		return PushSize;
	}
	// send
#if defined (X_SYSTEM_LINUX)
	ssize_t WB = send(TcpConnectionPtr->_Socket, DataPtr, Size, XelNoWriteSignal);
	X_DbgInfo("SendData: %zi", (size_t)WB);
	if (WB < 0) {
        if (errno != EAGAIN) {
			XTC_Close(TcpConnectionPtr);
			return (size_t)(-1);
		}
	}
	else {
		DataPtr += WB;
		Size -= WB;
	}

	if (Size) {
		XIEB_MarkWriting(&TcpConnectionPtr->_IoEventBase);
		size_t PushSize = XWBC_PushBack(WriteBufferChainPtr, DataPtr, Size);
		TcpConnectionPtr->_WriteBufferDataSize += PushSize;
		size_t Total = (size_t)WB + PushSize;
		return Total;
	}
	return WB;
#endif

	X_FatalAbort("XTC_PostData not implemented");
	return 0;
}

// Refs to IoContext 
//
// typedef void (*XelIoUserEventProc)(XelIoUserEvent * UserEventNodePtr);
// struct XelIoUserEvent {
//     XelIoUserEventProc  UserEventProc;
//     XelVariable         UserEventContext;
//     XelListNode         UserEventNode;
// };
static void PreprocessRemainReadings(XelIoUserEvent * TcpConnectionEventNodePtr)
{
	XelTcpConnection * TcpConnectionPtr = TcpConnectionEventNodePtr->UserEventContext.Ptr;
	XTC_EventCallback(&TcpConnectionPtr->_IoEventBase, XIET_In);
}

void XTC_SuspendReading(XelTcpConnection * TcpConnectionPtr)
{
	XIEB_SuspendReading(&TcpConnectionPtr->_IoEventBase);
}

void XTC_ResumeReading(XelTcpConnection * TcpConnectionPtr)
{
	XIEB_ResumeReading(&TcpConnectionPtr->_IoEventBase);
	if (TcpConnectionPtr->_ReadDataSize && TcpConnectionPtr->_IoEventBase._IoContextPtr) {
		XelVariable ContextVar = { .Ptr = TcpConnectionPtr};
		XIUE_SetEventProc(&TcpConnectionPtr->_ExtraIntenalEventNode, PreprocessRemainReadings, ContextVar);
		XIC_PushUserEvent(TcpConnectionPtr->_IoEventBase._IoContextPtr, &TcpConnectionPtr->_ExtraIntenalEventNode);
	}
}
