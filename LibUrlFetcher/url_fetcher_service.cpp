#include "stdafx.h"
#include "url_fetcher_service.h"
#include <thread>
#include <mutex>

namespace net
{


	URLFetcherService::URLFetcherService()
		: io_work_(NULL)
	{
		io_work_ = new asio::io_service::work(io_service_);
		std::thread t([=]()
		{
			io_service_.run();
		});
		t.detach();
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
		io_service_.reset();
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
		return new HttpRequestJob(io_service_, request, delegate);
	}

}