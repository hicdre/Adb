#include "stdafx.h"
#include "url_fetcher_impl.h"

namespace net
{
	

	URLFetcherImpl::URLFetcherImpl(const std::string& url
		, RequestType request_type
		, URLFetcherDelegate* d)
		: original_url_(url)
		, request_type_(request_type)
		, delegate_(d)
		, request_(NULL)
		, response_code_(URLFetcher::RESPONSE_CODE_INVALID)
		, was_cancelled_(false)
		, current_response_bytes_(0)
		, total_response_bytes_(-1)
	{

	}

	URLFetcherImpl::~URLFetcherImpl()
	{

	}

	void URLFetcherImpl::SetReferrer(const std::string& referrer)
	{
		referrer_ = referrer;
	}

	void URLFetcherImpl::SetExtraRequestHeaders(const std::string& extra_request_headers)
	{
		extra_request_headers_.Clear();
		extra_request_headers_.AddHeadersFromString(extra_request_headers);
	}

	void URLFetcherImpl::AddExtraRequestHeader(const std::string& header_line)
	{
		extra_request_headers_.AddHeadersFromString(header_line);
	}

	void URLFetcherImpl::SetStopOnRedirect(bool stop_on_redirect)
	{
		stop_on_redirect_ = stop_on_redirect;
	}

	void URLFetcherImpl::Start()
	{

	}

	const std::string& URLFetcherImpl::GetOriginalURL() const
	{
		return original_url_;
	}

	const std::string& URLFetcherImpl::GetURL() const
	{
		return url_;
	}

	const URLRequestStatus& URLFetcherImpl::GetStatus() const
	{
		return status_;
	}

	int URLFetcherImpl::GetResponseCode() const
	{
		return response_code_;
	}

	void URLFetcherImpl::ReceivedContentWasMalformed()
	{

	}

	bool URLFetcherImpl::GetResponseAsString(std::string* out_response_string) const
	{
		return false;
	}

}