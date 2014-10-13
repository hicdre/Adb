#include "stdafx.h"
#include "http_request.h"

namespace net
{


	HttpRequest::HttpRequest(const std::string& url)
		: url_(url)
		, method_(GET)
	{

	}

	HttpRequest::~HttpRequest()
	{

	}

	bool HttpRequest::IsValid() const
	{
		return url_.IsValid();
	}

	void HttpRequest::SetExtraHeaders(const std::string& extra_request_headers)
	{
		extra_request_headers_.Clear();
		extra_request_headers_.AddHeadersFromString(extra_request_headers);
	}

	void HttpRequest::AddExtraHeader(const std::string& header_line)
	{
		extra_request_headers_.AddHeadersFromString(header_line);
	}

	void HttpRequest::WriteToString(std::string* contents)
	{
		if (method_ == GET)
			contents->append("GET ");
		else if (method_ == POST)
			contents->append("POST ");
		contents->append(url_.path());
		contents->append(" HTTP/1.1\r\n");

		contents->append("Host: ");
		contents->append(url_.host());
		contents->append("\r\n");

		contents->append("Accept: */*\r\nConnection: close\r\n");

		if (!extra_request_headers_.IsEmpty())
			contents->append(extra_request_headers_.ToString());

		contents->append("\r\n");
	}

}