#include "stdafx.h"
#include "tcp_client_socket.h"

#include <MSWSock.h>

namespace net
{
	TCPClientSocket::State::State(TCPClientSocket* channel)
	{
		memset(&context.overlapped, 0, sizeof(context.overlapped));
		context.handler = channel;
	}

	TCPClientSocket::State::~State()
	{

	}

	TCPClientSocket::TCPClientSocket(base::IOThread* t)
		: io_(t)
		, delegate_(NULL)
		, socket_(NULL)
		, connect_state_(this)
	{
		Init();
	}

	TCPClientSocket::~TCPClientSocket()
	{
		if (socket_ && socket_ != INVALID_SOCKET)
		{
			closesocket(socket_);
		}
	}

	void TCPClientSocket::SetDelegate(Delegate* delegate)
	{
		delegate_ = delegate;
	}

	bool TCPClientSocket::Connect(const std::string& address, unsigned short port)
	{
		GUID gid = WSAID_CONNECTEX;
		LPFN_CONNECTEX ConnectEx = NULL;
		DWORD dwSize = 0;
		if (WSAIoctl(socket_, SIO_GET_EXTENSION_FUNCTION_POINTER, &gid, sizeof(gid), 
			&ConnectEx, sizeof(ConnectEx), &dwSize, NULL, NULL) != 0)
		{
			return false;
		}

		sock_addr_.sin_addr.s_addr = inet_addr(address.c_str());
		sock_addr_.sin_family = AF_INET;
		sock_addr_.sin_port = htons(port);

		DWORD dwLenth = 0;
		if (!ConnectEx(socket_, (sockaddr *)&sock_addr_, sizeof(sock_addr_), 
			NULL, 0, &dwLenth, &connect_state_.context.overlapped))
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				return false;
			}

		}
		return true;
		
	}

	void TCPClientSocket::Init()
	{
		socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socket_ == INVALID_SOCKET)
			return;

		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(SOCKADDR_IN));
		addr.sin_family = AF_INET;
		if (::bind(socket_, (sockaddr *)&addr, sizeof(SOCKADDR_IN)) != 0)
			return;

		io_->RegisterIOHandler((HANDLE)socket_, this);
	}

	void TCPClientSocket::OnIOCompleted(base::IOThread::IOContext* context, DWORD bytes_transfered, DWORD error)
	{

	}


	

}