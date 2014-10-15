#pragma once
#include "http_request.h"
#include "http_request_job.h"
#include "url_fetcher.h"
#include "url_fetcher_service.h"
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
	net::URLFetcherService service;
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
			client.GetUrl("http://211.162.73.115/PASV/BE5FC5DE6D834347B0728363F780C7DA/211.162.73.113/");
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}

	}
}