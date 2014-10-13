#pragma once
#include "http_request.h"
#include "http_request_job.h"
#include "url_fetcher.h"

void RunHttpClient()
{

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
			net::URLFetcher* req = net::URLFetcher::Create(
				"http://www.baidu.com/",
				net::URLFetcher::GET, NULL);

			req->Start();
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}

	}

}