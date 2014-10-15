#pragma once
#include "url_fetcher.h"
#include "http_request_headers.h"
#include "http_request_job.h"
#include "http_response_headers.h"
#include "http_response_writer.h"

namespace net
{
	class URLFetcherImpl 
		: public URLFetcher
		, public HttpRequestJob::Delegate
	{
	public:
		URLFetcherImpl(const std::string& url,
			RequestType request_type,
			URLFetcherDelegate* d);
		virtual ~URLFetcherImpl();

		virtual void SetReferrer(const std::string& referrer) override;
		virtual void SetExtraRequestHeaders(
			const std::string& extra_request_headers) override;
		virtual void AddExtraRequestHeader(const std::string& header_line) override;
		virtual void SetStopOnRedirect(bool stop_on_redirect) override;
		virtual void Start() override;
		virtual const std::string& GetOriginalURL() const override;
		virtual const URL& GetURL() const override;
		virtual const URLRequestStatus& GetStatus() const override;
		virtual int GetResponseCode() const override;
		virtual bool GetResponseAsString(std::string* out_response_string) const override;

		virtual void OnError(HttpRequestJob* job, const asio::error_code& err) override;
		virtual void OnRedirectUrl(HttpRequestJob* job, const URL& url) override;

		virtual void OnReceivedHeaders(HttpRequestJob* job, scoped_refptr<HttpResponseHeaders> headers) override;
		virtual void OnReceiveContents(HttpRequestJob* job, const char* data, std::size_t len) override;
		virtual void OnReceiveComplete(HttpRequestJob* job) override;

	private:
		void HandleRedirectLocation(const std::string& location);
		std::string original_url_;                // The URL we were asked to fetch
		URL url_;                         // The URL we eventually wound up at
		URLFetcher::RequestType request_type_;  // What type of request is this?
		URLRequestStatus status_;          // Status of the request
		URLFetcherDelegate* delegate_;     // Object to notify on completion
		HttpRequest* request_;
		scoped_refptr<HttpRequestJob> job_;

		
		int response_code_;                // HTTP status code for the request
		scoped_refptr<HttpResponseHeaders> responce_headers_;
		scoped_ptr<HttpResponseWriter> response_;

		bool was_cancelled_;
		// Number of bytes received so far.
		int64 current_response_bytes_;
		// Total expected bytes to receive (-1 if it cannot be determined).
		int64 total_response_bytes_;

		//config option
		bool stop_on_redirect_{ false };
		int redirect_count_;


	};
}