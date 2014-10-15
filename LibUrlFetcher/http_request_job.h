#pragma once
#include <functional>
#include "asio.hpp"
#include "basictypes.h"
#include "scoped_ptr.h"
#include "http_response_headers.h"
#include "url.h"

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

			virtual void OnReceivedHeaders(HttpRequestJob* job, scoped_refptr<HttpResponseHeaders> headers) {}

			virtual void OnRedirectUrl(HttpRequestJob* job, const URL& url) {}

			virtual void OnReceiveContents(HttpRequestJob* job, const char* data, std::size_t len) {}

			virtual void OnReceiveComplete(HttpRequestJob* job) {}
		};
		HttpRequestJob(asio::io_service& io_service,
			HttpRequest* request, Delegate* delegate);
		~HttpRequestJob();

		void AddRef();
		void Release();

		void Start();

		void Redirect(const URL& url);

		void Cancel();
	private:
		void HandleRedirect(URL url);

		void HandleResolve(const asio::error_code& err,
			asio::ip::tcp::resolver::iterator endpoint_iterator);

		void HandleConnect(const asio::error_code& err);

		void HandleWriteRequest(const asio::error_code& err);

		void HandleReadHeaders(const asio::error_code& err, std::size_t len);

		void HandleReadContent(const asio::error_code& err, std::size_t len);

		bool is_canceled() const { return cancel_; }
		asio::ip::tcp::resolver resolver_;
		asio::ip::tcp::socket socket_;
		std::string request_string_;
		asio::streambuf response_;

		HttpRequest* request_;
		Delegate* delegate_;

		bool cancel_;
		LONG ref_count_;
		//responce
		scoped_refptr<HttpResponseHeaders> response_headers_;
	};
}