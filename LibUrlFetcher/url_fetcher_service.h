#pragma once
#include "http_request_job.h"
#include <thread>

namespace net
{
	class URLFetcherService
	{
	public:
		URLFetcherService();
		~URLFetcherService();

		static URLFetcherService* Get();

		void Cancel();

		asio::io_service& service() { return *io_service_; }

		HttpRequestJob* CreateRequestJob(
			HttpRequest* request,
			HttpRequestJob::Delegate* delegate);

	private:
		std::shared_ptr<asio::io_service> io_service_;
		std::shared_ptr<asio::io_service::work> io_work_;
		std::thread* thread_;
	};
}