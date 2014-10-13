#pragma once
#include <functional>
#include "asio.hpp"

namespace net
{
	class HttpRequest;
	class HttpRequestJob
	{
	public:
		class Delegate {
		public:
			virtual ~Delegate() {}

			virtual void OnError(HttpRequestJob* job, const asio::error_code& err) {};
		};
		HttpRequestJob(asio::io_service& io_service,
			HttpRequest* request, Delegate* delegate);

		void Start();
	private:
		void HandleResolve(const asio::error_code& err,
			asio::ip::tcp::resolver::iterator endpoint_iterator);

		void HandleConnect(const asio::error_code& err);

		void HandleWriteRequest(const asio::error_code& err);

		void HandleReadStatusLine(const asio::error_code& err);

		void HandleReadHeaders(const asio::error_code& err);

		void HandleReadContent(const asio::error_code& err);

		asio::ip::tcp::resolver resolver_;
		asio::ip::tcp::socket socket_;
		std::string request_string_;
		asio::streambuf response_;

		HttpRequest* request_;
		Delegate* delegate_;
	};
}