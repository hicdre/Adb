#include "stdafx.h"
#include "url_fetcher_service.h"
#include <thread>
#include <mutex>

namespace net
{


	URLFetcherService::URLFetcherService()
		: io_work_(NULL)
	{
		io_service_ = new asio::io_service;
		io_work_ = new asio::io_service::work(*io_service_);
		thread_ = new std::thread([this]()
		{
			io_service_->run();
		});
	}

	URLFetcherService::~URLFetcherService()
	{
		Cancel();
	}

	void URLFetcherService::Cancel()
	{
		if (io_work_)
			delete io_work_;
		io_work_ = NULL;
		if (io_service_)
			io_service_->stop();
		if (thread_)
			thread_->join();
		if (io_service_)
			delete io_service_;
		io_service_ = NULL;
		if (thread_)
			delete thread_;
		thread_ = NULL;
		
	}

	URLFetcherService* URLFetcherService::Get()
	{
		static URLFetcherService* service = NULL;
		static std::once_flag flag;
		std::call_once(flag, []()
		{
			service = new URLFetcherService;
			atexit([]()
			{
				delete service;
				service = NULL;
			});
		});
		return service;
	}

	HttpRequestJob* URLFetcherService::CreateRequestJob(
		HttpRequest* request, 
		HttpRequestJob::Delegate* delegate)
	{
		return new HttpRequestJob(*io_service_, request, delegate);
	}

}