#include "stdafx.h"
#include "url.h"
#include <regex>

namespace net
{


	URL::URL()
		: valid_(false)
	{

	}

	URL::URL(const std::string& url)
		: valid_(false)
	{
		ParseURL(url);
	}

	URL::~URL()
	{

	}

	void URL::ParseURL(const std::string& url)
	{
		if (url.empty())
			return;

		std::regex pattern(R"(^http://([a-zA-Z0-9\.\-]+)(/[a-zA-Z0-9\.=-_]*)$)");
		std::match_results<std::string::const_iterator> result;
		if (!std::regex_match(url, result, pattern))
			return;
		
		host_ = result[1];
		if (result.size() == 2)
			path_ = "\\";
		else
			path_ = result[2];

		valid_ = true;
	}

}