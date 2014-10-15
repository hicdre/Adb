#include "stdafx.h"
#include "url_fetcher_service.h"
#include <thread>
#include <mutex>

namespace net
{
	namespace
	{
		static void io_service_run(std::shared_ptr<asio::io_service> io)
		{
			io->run();
		}

		static URLFetcherService* g_service = NULL;
	}

	URLFetcherService::URLFetcherService()
		: io_work_(NULL)
	{
		io_service_.reset(new asio::io_service);
		io_work_.reset(new asio::io_service::work(*io_service_));
		thread_ = new std::thread(std::bind(io_service_run, io_service_));

		g_service = this;
	}

	URLFetcherService::~URLFetcherService()
	{
		Cancel();

		g_service = NULL;
	}

	void URLFetcherService::Cancel()
	{
		io_work_.reset();
		io_service_->stop();
		if (thread_) {
			thread_->join();
			delete thread_;
			thread_ = NULL;
		}
		
	}

	URLFetcherService* URLFetcherService::Get()
	{
		return g_service;
	}

	HttpRequestJob* URLFetcherService::CreateRequestJob(
		HttpRequest* request, 
		HttpRequestJob::Delegate* delegate)
	{
		return new HttpRequestJob(*io_service_, request, delegate);
	}

}