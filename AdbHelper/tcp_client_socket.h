#pragma once
#include <WinSock2.h>
#include <string>

#include "io_thread.h"

namespace net
{
	class TCPClientSocket : public base::IOThread::IOHandler
	{
	public:
		class Delegate{
		public:
			virtual ~Delegate() {}
		};
		TCPClientSocket(base::IOThread* t);
		~TCPClientSocket();

		void SetDelegate(Delegate* delegate);

		bool Connect(const std::string& address, unsigned short port);



	private:
		virtual void OnIOCompleted(base::IOThread::IOContext* context, DWORD bytes_transfered,
			DWORD error) override;

		void Init();

	private:
		struct State {
			explicit State(TCPClientSocket* channel);
			~State();
			base::IOThread::IOContext context;
			bool is_pending;
		};

		State connect_state_;

		Delegate* delegate_;
		base::IOThread* io_;

		SOCKET socket_;
		SOCKADDR_IN sock_addr_;
	};
}