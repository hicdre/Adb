#include "stdafx.h"
#include "http_request_headers.h"

namespace net
{
	const char HttpRequestHeaders::kGetMethod[] = "GET";
	const char HttpRequestHeaders::kAcceptCharset[] = "Accept-Charset";
	const char HttpRequestHeaders::kAcceptEncoding[] = "Accept-Encoding";
	const char HttpRequestHeaders::kAcceptLanguage[] = "Accept-Language";
	const char HttpRequestHeaders::kAuthorization[] = "Authorization";
	const char HttpRequestHeaders::kCacheControl[] = "Cache-Control";
	const char HttpRequestHeaders::kConnection[] = "Connection";
	const char HttpRequestHeaders::kContentLength[] = "Content-Length";
	const char HttpRequestHeaders::kContentType[] = "Content-Type";
	const char HttpRequestHeaders::kCookie[] = "Cookie";
	const char HttpRequestHeaders::kHost[] = "Host";
	const char HttpRequestHeaders::kIfModifiedSince[] = "If-Modified-Since";
	const char HttpRequestHeaders::kIfNoneMatch[] = "If-None-Match";
	const char HttpRequestHeaders::kIfRange[] = "If-Range";
	const char HttpRequestHeaders::kOrigin[] = "Origin";
	const char HttpRequestHeaders::kPragma[] = "Pragma";
	const char HttpRequestHeaders::kProxyAuthorization[] = "Proxy-Authorization";
	const char HttpRequestHeaders::kProxyConnection[] = "Proxy-Connection";
	const char HttpRequestHeaders::kRange[] = "Range";
	const char HttpRequestHeaders::kReferer[] = "Referer";
	const char HttpRequestHeaders::kUserAgent[] = "User-Agent";
	const char HttpRequestHeaders::kTransferEncoding[] = "Transfer-Encoding";


	HttpRequestHeaders::HttpRequestHeaders()
	{

	}

	HttpRequestHeaders::~HttpRequestHeaders()
	{

	}

	bool HttpRequestHeaders::GetHeader(const std::string& key, std::string* out) const
	{
		HeaderVector::const_iterator it = FindHeader(key);
		if (it == headers_.end())
			return false;
		out->assign(it->value);
		return true;
	}

	void HttpRequestHeaders::Clear()
	{
		headers_.clear();
	}

	void HttpRequestHeaders::SetHeader(const std::string& key, const std::string& value)
	{
		HeaderVector::iterator it = FindHeader(key);
		if (it != headers_.end())
			it->value.assign(value.data(), value.size());
		else
			headers_.push_back(Pair(key, value));
	}

	void HttpRequestHeaders::SetHeaderIfMissing(const std::string& key, const std::string& value)
	{
		HeaderVector::iterator it = FindHeader(key);
		if (it == headers_.end())
			headers_.push_back(Pair(key, value));
	}

	void HttpRequestHeaders::RemoveHeader(const std::string& key)
	{
		HeaderVector::iterator it = FindHeader(key);
		if (it != headers_.end())
			headers_.erase(it);
	}

	bool HttpRequestHeaders::AddHeaderFromString(const std::string& header_line)
	{
		assert(std::string::npos == header_line.find("\r\n"));

		const std::string::size_type key_end_index = header_line.find(":");
		if (key_end_index == std::string::npos) {
			assert(0);
			return false;
		}

		if (key_end_index == 0) {
			assert(0);
			return false;
		}

		const std::string header_key(header_line.data(), key_end_index);

		const std::string::size_type value_index = key_end_index + 1;

		if (value_index < header_line.size()) {
			std::string header_value(header_line.data() + value_index,
				header_line.size() - value_index);
			std::string::const_iterator header_value_begin =
				header_value.begin();
			std::string::const_iterator header_value_end =
				header_value.end();

			{//trim whitespace
				// leading whitespace
				while (header_value_begin < header_value_end &&
					(header_value_begin[0] == ' ' || header_value_begin[0] == '\t'))
					++header_value_end;

				// trailing whitespace
				while (header_value_begin < header_value_end &&
					((header_value_end[-1] == ' ' || header_value_end[-1] == '\t')))
					--header_value_end;
			}


			if (header_value_begin == header_value_end) {
				// Value was all LWS.
				SetHeader(header_key, "");
			}
			else {
				SetHeader(header_key,
					std::string(&*header_value_begin,
					header_value_end - header_value_begin));
			}
		}
		else if (value_index == header_line.size()) {
			SetHeader(header_key, "");
		}
		else {
			assert(0);
		}
		return true;
	}

	void HttpRequestHeaders::AddHeadersFromString(const std::string& headers)
	{
		std::string::size_type begin_index = 0;
		while (true) {
			const std::string::size_type end_index = headers.find("\r\n", begin_index);
			if (end_index == std::string::npos) {
				const std::string term = headers.substr(begin_index);
				if (!term.empty())
					AddHeaderFromString(term);
				return;
			}
			const std::string term = headers.substr(begin_index, end_index - begin_index);
			if (!term.empty())
				AddHeaderFromString(term);
			begin_index = end_index + 2;
		}
	}

	void HttpRequestHeaders::MergeFrom(const HttpRequestHeaders& other)
	{
		for (HeaderVector::const_iterator it = other.headers_.begin();
			it != other.headers_.end(); ++it) {
			SetHeader(it->key, it->value);
		}
	}

	std::string HttpRequestHeaders::ToString() const
	{
		std::string output;
		for (HeaderVector::const_iterator it = headers_.begin();
			it != headers_.end(); ++it) {
			if (!it->value.empty()) {
				output.append(it->key);
				output.append(": ");
				output.append(it->value);
				output.append("\r\n");
			}
			else {
				output.append(it->key);
				output.append("\r\n");
			}
		}
		output.append("\r\n");
		return output;
	}

	HttpRequestHeaders::HeaderVector::const_iterator HttpRequestHeaders::FindHeader(const std::string& key) const
	{
		for (HeaderVector::const_iterator it = headers_.begin();
			it != headers_.end(); ++it) {
			if (key.length() == it->key.length() &&
				!_strnicmp(key.data(), it->key.data(), key.length()))
				return it;
		}

		return headers_.end();
	}

	HttpRequestHeaders::HeaderVector::iterator HttpRequestHeaders::FindHeader(const std::string& key)
	{
		for (HeaderVector::iterator it = headers_.begin();
			it != headers_.end(); ++it) {
			if (key.length() == it->key.length() &&
				!_strnicmp(key.data(), it->key.data(), key.length()))
				return it;
		}

		return headers_.end();
	}

}