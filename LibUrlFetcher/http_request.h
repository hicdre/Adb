#pragma once
#include "http_request_headers.h"
#include "url.h"
namespace net
{
	class HttpRequest
	{
	public:
		enum Method {
			GET,
			POST,
		};
		HttpRequest(const std::string& url);
		~HttpRequest();

		bool IsValid() const;
		void SetUrl(const URL& url);

		const URL& GetUrl() const { return url_; }

		void SetMethod(Method m) { method_ = m; }
		Method method() const { return method_; }

		void SetExtraHeaders(const std::string& extra_request_headers);
		void AddExtraHeader(const std::string& header_line);

		void WriteToString(std::string* contents);
	private:
		URL url_;
		Method method_;

		HttpRequestHeaders extra_request_headers_;
	};
}