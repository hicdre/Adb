#include "stdafx.h"
#include "http_response_writer.h"

namespace net
{


	HttpResponseStringWriter::HttpResponseStringWriter()
	{

	}

	HttpResponseStringWriter::~HttpResponseStringWriter()
	{

	}

	void HttpResponseStringWriter::Init()
	{
		contents_.clear();
	}

	void HttpResponseStringWriter::Write(const char* buffer, int num_bytes)
	{
		contents_.append(buffer, num_bytes);
	}

	void HttpResponseStringWriter::Finish()
	{

	}

	HttpResponseStringWriter* HttpResponseStringWriter::GetAsStringWriter()
	{
		return this;
	}

	const std::string& HttpResponseStringWriter::GetString() const
	{
		return contents_;
	}

}