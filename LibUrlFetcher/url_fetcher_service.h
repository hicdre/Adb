#pragma once
#include "http_request_job.h"

namespace net
{
	class URLFetcherService
	{
	public:
		static URLFetcherService* Get();

		void Cancel();

		asio::io_service& service() { return io_service_; }

		HttpRequestJob* CreateRequestJob(
			HttpRequest* request,
			HttpRequestJob::Delegate* delegate);

	private:
		URLFetcherService();
		~URLFetcherService();

		asio::io_service io_service_;
		asio::io_service::work* io_work_;
	};
}