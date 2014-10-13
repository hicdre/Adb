#pragma once
#include "asio.hpp"
#include "basictypes.h"
#include "url_request_status.h"

namespace net
{
	class URLFetcher;

	// A delegate interface for users of URLFetcher.
	class URLFetcherDelegate
	{
	public:
		virtual ~URLFetcherDelegate() {}
		// This will be called when the URL has been fetched, successfully or not.
		// Use accessor methods on |source| to get the results.
		virtual void OnURLFetchComplete(const URLFetcher* source) = 0;
		virtual void OnURLFetchDownloadProgress(const URLFetcher* source,
			int64 current, int64 total) {}

		virtual void OnURLFetchUploadProgress(const URLFetcher* source,
			int64 current, int64 total) {}
	};

	class URLFetcher
	{
	public:
		enum ResponseCode {
			RESPONSE_CODE_INVALID = -1
		};

		enum RequestType {
			GET,
			POST,
			HEAD,
			DELETE_REQUEST,   // DELETE is already taken on Windows.
			// <winnt.h> defines a DELETE macro.
			PUT,
			PATCH,
		};

		virtual ~URLFetcher() {}

		static URLFetcher* Create(const std::string& url,
			URLFetcher::RequestType request_type,
			URLFetcherDelegate* d);

		static void CancelAll();

		virtual void SetReferrer(const std::string& referrer) = 0;

		virtual void SetExtraRequestHeaders(
			const std::string& extra_request_headers) = 0;

		virtual void AddExtraRequestHeader(const std::string& header_line) = 0;

		virtual void SetStopOnRedirect(bool stop_on_redirect) = 0;

		virtual void Start() = 0;

		virtual const std::string& GetOriginalURL() const = 0;

		// Return the URL that this fetcher is processing.
		virtual const std::string& GetURL() const = 0;

		// The status of the URL fetch.
		virtual const URLRequestStatus& GetStatus() const = 0;

		// The http response code received. Will return RESPONSE_CODE_INVALID
		// if an error prevented any response from being received.
		virtual int GetResponseCode() const = 0;

		// Cookies received.
		//virtual const ResponseCookies& GetCookies() const = 0;

		// Reports that the received content was malformed.
		virtual void ReceivedContentWasMalformed() = 0;

		// Get the response as a string. Return false if the fetcher was not
		// set to store the response as a string.
		virtual bool GetResponseAsString(std::string* out_response_string) const = 0;
	};


}