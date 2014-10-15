#include "stdafx.h"
#include "url_fetcher_impl.h"
#include "http_request.h"
#include "url_fetcher_service.h"

#include <iostream>

namespace net
{

	URLFetcher* URLFetcher::Create(
		const std::string& url, 
		URLFetcher::RequestType request_type, 
		URLFetcherDelegate* d)
	{
		return new URLFetcherImpl(url, request_type, d);
	}


	URLFetcherImpl::URLFetcherImpl(const std::string& url
		, RequestType request_type
		, URLFetcherDelegate* d)
		: original_url_(url)
		, request_type_(request_type)
		, delegate_(d)
		, request_(new HttpRequest(url))
		, response_code_(URLFetcher::RESPONSE_CODE_INVALID)
		, was_cancelled_(false)
		, current_response_bytes_(0)
		, total_response_bytes_(-1)
		, redirect_count_(0)
	{

	}

	URLFetcherImpl::~URLFetcherImpl()
	{
		if (job_.get()) {
			job_->Cancel();
		}
	}

	void URLFetcherImpl::SetReferrer(const std::string& referrer)
	{
		//referrer_ = referrer;
	}

	void URLFetcherImpl::SetExtraRequestHeaders(const std::string& extra_request_headers)
	{
		request_->SetExtraHeaders(extra_request_headers);
	}

	void URLFetcherImpl::AddExtraRequestHeader(const std::string& header_line)
	{
		request_->AddExtraHeader(header_line);
	}

	void URLFetcherImpl::SetStopOnRedirect(bool stop_on_redirect)
	{
		stop_on_redirect_ = stop_on_redirect;
	}

	void URLFetcherImpl::Start()
	{
		if (status_.is_io_pending())
			return;

		status_.set_status(URLRequestStatus::IO_PENDING);
		request_->SetMethod((HttpRequest::Method)request_type_);
		if (!response_.get())
			response_.reset(new HttpResponseStringWriter);

		job_.reset(URLFetcherService::Get()->CreateRequestJob(request_, this));

		job_->Start();
	}

	const std::string& URLFetcherImpl::GetOriginalURL() const
	{
		return original_url_;
	}

	const URL& URLFetcherImpl::GetURL() const
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

	bool URLFetcherImpl::GetResponseAsString(std::string* out_response_string) const
	{
		HttpResponseStringWriter* string_response = response_->GetAsStringWriter();
		if (!string_response)
			return false;

		*out_response_string = string_response->GetString();
		return true;
	}

	void URLFetcherImpl::OnError(HttpRequestJob* job, const asio::error_code& err)
	{
		status_.set_status(URLRequestStatus::FAILED);
		status_.set_error(err.value());
		if (delegate_)
			delegate_->OnURLFetchComplete(this);
	}

	void URLFetcherImpl::OnReceivedHeaders(HttpRequestJob* job, scoped_refptr<HttpResponseHeaders> headers)
	{
		responce_headers_ = headers;
		total_response_bytes_ = responce_headers_->GetContentLength();
		current_response_bytes_ = 0;
		response_code_ = responce_headers_->response_code();

		std::string location;
		if (responce_headers_->IsRedirect(&location)) {
			if (!stop_on_redirect_) {
				HandleRedirectLocation(location);
				return;
			}

			job->Cancel();
			return;
		}
		response_->Init();
	}

	void URLFetcherImpl::OnReceiveContents(HttpRequestJob* job, const char* data, std::size_t len)
	{
		current_response_bytes_ += len;

		response_->Write(data, len);

		if (delegate_)
			delegate_->OnURLFetchDownloadProgress(this, current_response_bytes_, total_response_bytes_);
	}

	void URLFetcherImpl::OnReceiveComplete(HttpRequestJob* job)
	{
		status_.set_status(URLRequestStatus::SUCCESS);
		if (delegate_)
			delegate_->OnURLFetchComplete(this);

		//ÇåÀísocket
		job_.reset();
	}

	void URLFetcherImpl::HandleRedirectLocation(const std::string& location)
	{
		URL url;
		if (!_strnicmp(location.c_str(), "http://", 7))
		{
			url = location;
		}
		else if (location[0] == '/')
		{
			url.set_path(location);
		}
		else
		{
			std::string origin_path = url_.path();
			std::string::size_type pos = origin_path.find_last_of('/');
			assert(pos != std::string::npos);
			url.set_path(origin_path.substr(0, pos + 1) + location);
		}

		job_->Redirect(url);
	}

	void URLFetcherImpl::OnRedirectUrl(HttpRequestJob* job, const URL& url)
	{
		static int kRedirectMax = 5;
		url_ = url;
		redirect_count_++;
		if (redirect_count_ >= 5) {
			job->Cancel();
		}
	}

}