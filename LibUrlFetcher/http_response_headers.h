#pragma once
#include "basictypes.h"
#include <unordered_set>

namespace net
{
	class HttpResponseHeaders
	{
	public:
		HttpResponseHeaders(const std::string& raw_headers);

		void AddRef();
		void Release();

		void Update(const HttpResponseHeaders& new_headers);		
		void RemoveHeader(const std::string& name);
		void RemoveHeaderLine(const std::string& name, const std::string& value);
		// Add a single header
		void AddHeader(const std::string& header);
		void ReplaceStatusLine(const std::string& new_status);

		std::string GetStatusLine() const;

		bool EnumerateHeaderLines(void** iter, 
			std::string* name, 
			std::string* value) const;

		bool EnumerateHeader(void** iter, 
			const std::string& name, 
			std::string* value) const;

		bool HasHeaderValue(const std::string& name,
			const std::string& value) const;

		bool HasHeader(const std::string& name) const;

		bool IsRedirect(std::string* location) const;

		static bool IsRedirectResponseCode(int response_code);

		int64 GetContentLength() const;

		// Extracts the value of the specified header or returns -1 if there is no
		// such header in the response.
		int64 GetInt64HeaderValue(const std::string& header) const;

		bool GetContentRange(int64* first_byte_position,
			int64* last_byte_position,
			int64* instance_length) const;

		int response_code() const { return response_code_; }

		// Returns the raw header string.
		const std::string& raw_headers() const { return raw_headers_; }

	private:
		struct ParsedHeader;
		typedef std::vector<ParsedHeader> HeaderList;
		typedef std::unordered_set<std::string> HeaderSet;

		HttpResponseHeaders();
		~HttpResponseHeaders();

		void Parse(const std::string& raw_input);

		void ParseStatusLine(std::string::const_iterator line_begin,
			std::string::const_iterator line_end,
			bool has_headers);

		// Find the header in our list (case-insensitive) starting with parsed_ at
		// index |from|.  Returns string::npos if not found
		size_t FindHeader(size_t from, const std::string& name) const;

		void AddHeader(std::string::const_iterator name_begin,
			std::string::const_iterator name_end,
			std::string::const_iterator value_begin,
			std::string::const_iterator value_end);

		void AddToParsed(std::string::const_iterator name_begin,
			std::string::const_iterator name_end,
			std::string::const_iterator value_begin,
			std::string::const_iterator value_end);

		// Replaces the current headers with the merged version of |raw_headers| and
		// the current headers without the headers in |headers_to_remove|. Note that
		// |headers_to_remove| are removed from the current headers (before the
		// merge), not after the merge.
		void MergeWithHeaders(const std::string& raw_headers,
			const HeaderSet& headers_to_remove);

		HeaderList parsed_;

		std::string raw_headers_;

		int response_code_;

		LONG ref_count_;

		DISALLOW_COPY_AND_ASSIGN(HttpResponseHeaders);
	};
}