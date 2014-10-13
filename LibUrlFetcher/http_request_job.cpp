#include "stdafx.h"
#include "http_request_job.h"
#include "http_request.h"
#include <iostream>

namespace net
{


	HttpRequestJob::HttpRequestJob(asio::io_service& io_service, 
		HttpRequest* request, Delegate* delegate)
		: resolver_(io_service)
		, socket_(io_service)
		, request_(request)
		, delegate_(delegate)
	{
		
	}


	void HttpRequestJob::Start()
	{
		asio::ip::tcp::resolver::query query(request_->GetUrl().host(), "http");
		resolver_.async_resolve(query,
			std::bind(&HttpRequestJob::HandleResolve, this,
			std::placeholders::_1,
			std::placeholders::_2));
	}


	void HttpRequestJob::HandleResolve(const asio::error_code& err, asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		if (!err)
		{
			// Attempt a connection to each endpoint in the list until we
			// successfully establish a connection.
			asio::async_connect(socket_, endpoint_iterator,
				std::bind(&HttpRequestJob::HandleConnect, this,
				std::placeholders::_1));
		}
		else
		{
			if (delegate_)
				delegate_->OnError(this, err);
		}
	}

	void HttpRequestJob::HandleConnect(const asio::error_code& err)
	{
		if (!err)
		{
			request_string_.clear();
			request_->WriteToString(&request_string_);
			// The connection was successful. Send the request.
			asio::async_write(socket_, 
				asio::buffer(request_string_.c_str(), request_string_.size()),
				std::bind(&HttpRequestJob::HandleWriteRequest, this,
				std::placeholders::_1));
		}
		else
		{
			if (delegate_)
				delegate_->OnError(this, err);
		}
	}

	void HttpRequestJob::HandleWriteRequest(const asio::error_code& err)
	{
		if (!err)
		{
			// Read the response status line. The response_ streambuf will
			// automatically grow to accommodate the entire line. The growth may be
			// limited by passing a maximum size to the streambuf constructor.
			asio::async_read_until(socket_, response_, "\r\n",
				std::bind(&HttpRequestJob::HandleReadStatusLine, this,
				std::placeholders::_1));
		}
		else
		{
			if (delegate_)
				delegate_->OnError(this, err);
		}
	}

	void HttpRequestJob::HandleReadStatusLine(const asio::error_code& err)
	{
		if (!err)
		{
			// Check that response is OK.
			std::istream response_stream(&response_);
			std::string http_version;
			response_stream >> http_version;
			response_stream >> status_code_;
			std::string status_message;
			std::getline(response_stream, status_message);
			if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			{
				std::cout << "Invalid response\n";
				return;
			}
			delegate_->OnGetStatus(this, status_code_);

			// Read the response headers, which are terminated by a blank line.
			asio::async_read_until(socket_, response_, "\r\n\r\n",
				std::bind(&HttpRequestJob::HandleReadHeaders, this,
				std::placeholders::_1));
		}
		else
		{
			std::cout << "Error: " << err << "\n";
		}
	}

	void HttpRequestJob::HandleReadHeaders(const asio::error_code& err)
	{
		if (!err)
		{
			// Process the response headers.
			std::istream response_stream(&response_);
			std::string header;
			while (std::getline(response_stream, header) && header != "\r")
				std::cout << header << "\n";
			std::cout << "\n";

			// Write whatever content we already have to output.
			if (response_.size() > 0)
				std::cout << &response_;

			if (status_code_ != 200)
			{
				socket_.get_io_service()
					.post(std::bind(&HttpRequestJob::HandleComplete, this));
			}

			// Start reading remaining data until EOF.
			current_response_bytes_ = 0;
			total_response_bytes_ = 0;
			asio::async_read(socket_, response_,
				asio::transfer_at_least(1),
				std::bind(&HttpRequestJob::HandleReadContent, this,
				std::placeholders::_1,
				std::placeholders::_2));
		}
		else
		{
			std::cout << "Error: " << err << "\n";
		}
	}

	void HttpRequestJob::HandleReadContent(const asio::error_code& err, std::size_t len)
	{
		if (!err)
		{
			// Write all of the data that has been read so far.
			current_response_bytes_ += len;
			std::cout << &response_;

			// Continue reading remaining data until EOF.
			asio::async_read(socket_, response_,
				asio::transfer_at_least(1),
				std::bind(&HttpRequestJob::HandleReadContent, this,
				std::placeholders::_1,
				std::placeholders::_2));
		}
		else if (err != asio::error::eof)
		{
			std::cout << "Error: " << err << "\n";
		}
	}

	void HttpRequestJob::HandleComplete()
	{

	}

}