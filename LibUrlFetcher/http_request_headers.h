#pragma once

namespace net
{
	class HttpRequestHeaders
	{
	public:
		struct Pair
		{
			Pair(const std::string& k, const std::string& v)
				: key(k), value(v) {}
			std::string key;
			std::string value;
		};

		typedef std::vector<Pair> HeaderVector;

		static const char kGetMethod[];

		static const char kAcceptCharset[];
		static const char kAcceptEncoding[];
		static const char kAcceptLanguage[];
		static const char kAuthorization[];
		static const char kCacheControl[];
		static const char kConnection[];
		static const char kContentType[];
		static const char kCookie[];
		static const char kContentLength[];
		static const char kHost[];
		static const char kIfModifiedSince[];
		static const char kIfNoneMatch[];
		static const char kIfRange[];
		static const char kOrigin[];
		static const char kPragma[];
		static const char kProxyAuthorization[];
		static const char kProxyConnection[];
		static const char kRange[];
		static const char kReferer[];
		static const char kUserAgent[];
		static const char kTransferEncoding[];

		HttpRequestHeaders();
		~HttpRequestHeaders();

		bool IsEmpty() const { return headers_.empty(); }

		bool HasHeader(const std::string& key) const {
			return FindHeader(key) != headers_.end();
		}

		// Gets the first header that matches |key|.  If found, returns true and
		// writes the value to |out|.
		bool GetHeader(const std::string& key, std::string* out) const;

		// Clears all the headers.
		void Clear();

		// Sets the header value pair for |key| and |value|.  If |key| already exists,
		// then the header value is modified, but the key is untouched, and the order
		// in the vector remains the same.  When comparing |key|, case is ignored.
		void SetHeader(const std::string& key, const std::string& value);

		// Sets the header value pair for |key| and |value|, if |key| does not exist.
		// If |key| already exists, the call is a no-op.
		// When comparing |key|, case is ignored.
		void SetHeaderIfMissing(const std::string& key,
			const std::string& value);

		// Removes the first header that matches (case insensitive) |key|.
		void RemoveHeader(const std::string& key);

		// Parses the header from a string and calls SetHeader() with it.  This string
		// should not contain any CRLF.  As per RFC2616, the format is:
		//
		// message-header = field-name ":" [ field-value ]
		// field-name     = token
		// field-value    = *( field-content | LWS )
		// field-content  = <the OCTETs making up the field-value
		//                  and consisting of either *TEXT or combinations
		//                  of token, separators, and quoted-string>
		//
		// AddHeaderFromString() will trim any LWS surrounding the
		// field-content.
		bool AddHeaderFromString(const std::string& header_line);

		// Same thing as AddHeaderFromString() except that |headers| is a "\r\n"
		// delimited string of header lines.  It will split up the string by "\r\n"
		// and call AddHeaderFromString() on each.
		void AddHeadersFromString(const std::string& headers);

		// Calls SetHeader() on each header from |other|, maintaining order.
		void MergeFrom(const HttpRequestHeaders& other);

		// Copies from |other| to |this|.
		void CopyFrom(const HttpRequestHeaders& other) {
			*this = other;
		}

		void Swap(HttpRequestHeaders* other) {
			headers_.swap(other->headers_);
		}

		// Serializes HttpRequestHeaders to a string representation.  Joins all the
		// header keys and values with ": ", and inserts "\r\n" between each header
		// line, and adds the trailing "\r\n".
		std::string ToString() const;

	private:
		HeaderVector::const_iterator FindHeader(const std::string& key) const;
		HeaderVector::iterator FindHeader(const std::string& key);
		HeaderVector headers_;
	};
}