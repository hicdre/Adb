#pragma once
#include "http_request.h"
#include "http_request_job.h"

void RunHttpClient()
{
	try
	{
		net::HttpRequest req("http://www.baidu.com/");
		req.SetMethod(net::HttpRequest::GET);

		asio::io_service io;
		asio::io_service::work work(io);

		net::HttpRequestJob job(io, &req, NULL);
		job.Start();

		io.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

}