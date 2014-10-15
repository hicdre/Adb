#pragma once
#include "http_request.h"
#include "http_request_job.h"
#include "url_fetcher.h"
#include <thread>

class HttpClient : public net::URLFetcherDelegate
{
public:
	~HttpClient()
	{
		if (fetcher_)
			delete fetcher_;
		fetcher_ = NULL;
	}
	void GetUrl(const std::string& url)
	{
		fetcher_ = net::URLFetcher::Create(url, net::URLFetcher::GET, this);

		fetcher_->Start();
	}
	virtual void OnURLFetchComplete(const net::URLFetcher* source) override
	{
		// in thread
		std::string str;
		if (fetcher_->GetResponseAsString(&str))
			std::cout << str;
	}

	net::URLFetcher* fetcher_{NULL};
};

void RunHttpClient()
{
	
	HttpClient client;
	while (true)
	{
		std::cout << "Enter message: ";

		char request[max_length];
		char tmp[5] = { 0 };
		std::cin.getline(request, max_length);
		//char* request = "host:version";

		if (!strncmp(request, "quit", 4))
			break;

		try
		{
			client.GetUrl("http://www.baidu.com/");
			//break;
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}

	}
}