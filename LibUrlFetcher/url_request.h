#pragma once

namespace net
{
	class UrlRequest
	{
	public:
		UrlRequest(const std::string& url);
		~UrlRequest();

		const std::string& method() const { return method_; }
		void set_method(const std::string& method);
	private:
		std::string method_;  // "GET", "POST", etc. Should be all uppercase.
	};
}