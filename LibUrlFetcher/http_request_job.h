#pragma once
#include <functional>
#include "asio.hpp"
#include "basictypes.h"

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

			virtual void OnGetStatus(HttpRequestJob* job, int status_code) {}

			virtual void OnGetHeaders(HttpRequestJob* job) {}
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

		void HandleReadContent(const asio::error_code& err, std::size_t len);

		void HandleComplete();

		asio::ip::tcp::resolver resolver_;
		asio::ip::tcp::socket socket_;
		std::string request_string_;
		asio::streambuf response_;

		HttpRequest* request_;
		Delegate* delegate_;

		//responce
		int status_code_;
		int64 current_response_bytes_;
		int64 total_response_bytes_;
	};
}