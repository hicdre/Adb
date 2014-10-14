#include "stdafx.h"
#include "http_response_headers.h"
#include "string_tokenizer.h"
#include "url.h"

namespace
{
	void StringToLower(std::string& str)
	{
		for (char& ch : str)
		{
			if ('A' <= ch && ch <= 'Z')
				ch += 'a' - 'A';
		}
	}

	bool IsLWS(char c) {
		return c == ' ' || c == '\t';
	}

	void TrimLWS(std::string::const_iterator* begin,
		std::string::const_iterator* end) {
		// leading whitespace
		while (*begin < *end && IsLWS((*begin)[0]))
			++(*begin);

		// trailing whitespace
		while (*begin < *end && IsLWS((*end)[-1]))
			--(*end);
	}

	bool IsNonCoalescingHeader(std::string::const_iterator name_begin,
		std::string::const_iterator name_end) {
		// NOTE: "set-cookie2" headers do not support expires attributes, so we don't
		// have to list them here.
		const char* kNonCoalescingHeaders[] = {
			"date",
			"expires",
			"last-modified",
			"location",  // See bug 1050541 for details
			"retry-after",
			"set-cookie",
			// The format of auth-challenges mixes both space separated tokens and
			// comma separated properties, so coalescing on comma won't work.
			"www-authenticate",
			"proxy-authenticate",
			// STS specifies that UAs must not process any STS headers after the first
			// one.
			"strict-transport-security"
		};
		for (size_t i = 0; i < arraysize(kNonCoalescingHeaders); ++i) {
			//if (LowerCaseEqualsASCII(name_begin, name_end, kNonCoalescingHeaders[i]))
			if (!_strnicmp(&name_begin[0], kNonCoalescingHeaders[i], name_end - name_begin))
				return true;
		}
		return false;
	}

	bool StringToInt64(std::string::const_iterator begin,
		std::string::const_iterator end, int64* val)
	{
		const char* begin_ptr = &begin[0];
		char* end_ptr = NULL;
		*val = _strtoi64(&begin[0], &end_ptr, 10);
		if (*val == 0 && end_ptr == begin_ptr)
			return false;
		return true;
	}

	int LocateStartOfStatusLine(const char* buf, int buf_len) {
		const int slop = 4;
		const int http_len = 4;

		if (buf_len >= http_len) {
			int i_max = (std::min)(buf_len - http_len, slop);
			for (int i = 0; i <= i_max; ++i) {
				if (!_strnicmp(buf + i, "http", http_len))
					return i;
			}
		}
		return -1;  // Not found
	}

	const char* FindStatusLineEnd(const char* begin, const char* end) {
		const char* pos = strstr(begin, "\r\n");
		if (pos)
			return pos + 1;
		return end;

// 		size_t i = base::StringPiece(begin, end - begin).find_first_of("\r\n");
// 		if (i == base::StringPiece::npos)
// 			return end;
// 		return begin + i;
	}

	const char* FindFirstNonLWS(const char* begin, const char* end) {
		for (const char* cur = begin; cur != end; ++cur) {
			if (!IsLWS(*cur))
				return cur;
		}
		return end;  // Not found.
	}

	bool IsLineSegmentContinuable(const char* begin, const char* end) {
		if (begin == end)
			return false;

		const char* colon = std::find(begin, end, ':');
		if (colon == end)
			return false;

		const char* name_begin = begin;
		const char* name_end = colon;

		// Name can't be empty.
		if (name_begin == name_end)
			return false;

		// Can't start with LWS (this would imply the segment is a continuation)
		if (IsLWS(*name_begin))
			return false;

		return true;
	}

	std::string AssembleRawHeaders(const char* input_begin,
		int input_len) {
		std::string raw_headers;
		raw_headers.reserve(input_len);

		const char* input_end = input_begin + input_len;

		// Skip any leading slop, since the consumers of this output
		// (HttpResponseHeaders) don't deal with it.
		int status_begin_offset = LocateStartOfStatusLine(input_begin, input_len);
		if (status_begin_offset != -1)
			input_begin += status_begin_offset;

		// Copy the status line.
		const char* status_line_end = FindStatusLineEnd(input_begin, input_end);
		raw_headers.append(input_begin, status_line_end);

		// After the status line, every subsequent line is a header line segment.
		// Should a segment start with LWS, it is a continuation of the previous
		// line's field-value.

		// TODO(ericroman): is this too permissive? (delimits on [\r\n]+)
		base::CStringTokenizer lines(status_line_end, input_end, "\r\n");

		// This variable is true when the previous line was continuable.
		bool prev_line_continuable = false;

		while (lines.GetNext()) {
			const char* line_begin = lines.token_begin();
			const char* line_end = lines.token_end();

			if (prev_line_continuable && IsLWS(*line_begin)) {
				// Join continuation; reduce the leading LWS to a single SP.
				raw_headers.push_back(' ');
				raw_headers.append(FindFirstNonLWS(line_begin, line_end), line_end);
			}
			else {
				// Terminate the previous line.
				raw_headers.push_back('\n');

				// Copy the raw data to output.
				raw_headers.append(line_begin, line_end);

				// Check if the current line can be continued.
				prev_line_continuable = IsLineSegmentContinuable(line_begin, line_end);
			}
		}

		raw_headers.append("\n\n", 2);

		// Use '\0' as the canonical line terminator. If the input already contained
		// any embeded '\0' characters we will strip them first to avoid interpreting
		// them as line breaks.
		raw_headers.erase(std::remove(raw_headers.begin(), raw_headers.end(), '\0'),
			raw_headers.end());
		std::replace(raw_headers.begin(), raw_headers.end(), '\n', '\0');

		return raw_headers;
	}
}

namespace net
{
	const char kContentRange[] = "Content-Range";

	struct HttpResponseHeaders::ParsedHeader {
		// A header "continuation" contains only a subsequent value for the
		// preceding header.  (Header values are comma separated.)
		bool is_continuation() const { return name_begin == name_end; }

		std::string::const_iterator name_begin;
		std::string::const_iterator name_end;
		std::string::const_iterator value_begin;
		std::string::const_iterator value_end;
	};

	HttpResponseHeaders::HttpResponseHeaders(const std::string& raw_headers)
		: response_code_(-1)
		, ref_count_(0)
	{
		Parse(AssembleRawHeaders(raw_headers.c_str(), raw_headers.size()));
	}

	HttpResponseHeaders::HttpResponseHeaders()
		: response_code_(-1)
		, ref_count_(0)
	{

	}

	HttpResponseHeaders::~HttpResponseHeaders()
	{

	}

// 	void HttpResponseHeaders::Update(const HttpResponseHeaders& new_headers)
// 	{
// 		assert(new_headers.response_code() == 304 ||
// 			new_headers.response_code() == 206);
// 	}


	void HttpResponseHeaders::MergeWithHeaders(const std::string& raw_headers, 
		const HeaderSet& headers_to_remove)
	{
		std::string new_raw_headers(raw_headers);
		for (size_t i = 0; i < parsed_.size(); ++i) {
			assert(!parsed_[i].is_continuation());

			// Locate the start of the next header.
			size_t k = i;
			while (++k < parsed_.size() && parsed_[k].is_continuation()) {}
			--k;

			std::string name(parsed_[i].name_begin, parsed_[i].name_end);
			StringToLower(name);
			if (headers_to_remove.find(name) == headers_to_remove.end()) {
				// It's ok to preserve this header in the final result.
				new_raw_headers.append(parsed_[i].name_begin, parsed_[k].value_end);
				new_raw_headers.push_back('\0');
			}

			i = k;
		}
		new_raw_headers.push_back('\0');

		// Make this object hold the new data.
		raw_headers_.clear();
		parsed_.clear();
		Parse(new_raw_headers);
	}


	void HttpResponseHeaders::RemoveHeader(const std::string& name)
	{
		// Copy up to the null byte.  This just copies the status line.
		std::string new_raw_headers(raw_headers_.c_str());
		new_raw_headers.push_back('\0');

		std::string lowercase_name(name);
		StringToLower(lowercase_name);
		HeaderSet to_remove;
		to_remove.insert(lowercase_name);
		MergeWithHeaders(new_raw_headers, to_remove);
	}

	void HttpResponseHeaders::RemoveHeaderLine(const std::string& name, 
		const std::string& value)
	{
		std::string name_lowercase(name);
		StringToLower(name_lowercase);

		std::string new_raw_headers(GetStatusLine());
		new_raw_headers.push_back('\0');

		new_raw_headers.reserve(raw_headers_.size());

		void* iter = NULL;
		std::string old_header_name;
		std::string old_header_value;
		while (EnumerateHeaderLines(&iter, &old_header_name, &old_header_value)) {
			std::string old_header_name_lowercase(name);
			StringToLower(old_header_name_lowercase);

			if (name_lowercase == old_header_name_lowercase &&
				value == old_header_value)
				continue;

			new_raw_headers.append(old_header_name);
			new_raw_headers.push_back(':');
			new_raw_headers.push_back(' ');
			new_raw_headers.append(old_header_value);
			new_raw_headers.push_back('\0');
		}
		new_raw_headers.push_back('\0');

		// Make this object hold the new data.
		raw_headers_.clear();
		parsed_.clear();
		Parse(new_raw_headers);
	}

	void HttpResponseHeaders::AddHeader(const std::string& header)
	{
		if (header.find('\0') != std::string::npos) {
			assert(0);
			return;
		}
			
		assert('\0' == raw_headers_[raw_headers_.size() - 2]);
		assert('\0' == raw_headers_[raw_headers_.size() - 1]);
		// Don't copy the last null.
		std::string new_raw_headers(raw_headers_, 0, raw_headers_.size() - 1);
		new_raw_headers.append(header);
		new_raw_headers.push_back('\0');
		new_raw_headers.push_back('\0');

		// Make this object hold the new data.
		raw_headers_.clear();
		parsed_.clear();
		Parse(new_raw_headers);
	}

	void HttpResponseHeaders::ReplaceStatusLine(const std::string& new_status)
	{
		if (new_status.find('\0') != std::string::npos) {
			assert(0);
			return;
		}
		std::string new_raw_headers(new_status);
		new_raw_headers.push_back('\0');

		HeaderSet empty_to_remove;
		MergeWithHeaders(new_raw_headers, empty_to_remove);
	}

	std::string HttpResponseHeaders::GetStatusLine() const
	{
		return std::string(raw_headers_.c_str());
	}

	bool HttpResponseHeaders::EnumerateHeaderLines(void** iter,
		std::string* name,
		std::string* value) const {
		size_t i = reinterpret_cast<size_t>(*iter);
		if (i == parsed_.size())
			return false;

		assert(!parsed_[i].is_continuation());

		name->assign(parsed_[i].name_begin, parsed_[i].name_end);

		std::string::const_iterator value_begin = parsed_[i].value_begin;
		std::string::const_iterator value_end = parsed_[i].value_end;
		while (++i < parsed_.size() && parsed_[i].is_continuation())
			value_end = parsed_[i].value_end;

		value->assign(value_begin, value_end);

		*iter = reinterpret_cast<void*>(i);
		return true;
	}

	bool HttpResponseHeaders::EnumerateHeader(void** iter,
		const std::string& name,
		std::string* value) const {
		size_t i;
		if (!iter || !*iter) {
			i = FindHeader(0, name);
		}
		else {
			i = reinterpret_cast<size_t>(*iter);
			if (i >= parsed_.size()) {
				i = std::string::npos;
			}
			else if (!parsed_[i].is_continuation()) {
				i = FindHeader(i, name);
			}
		}

		if (i == std::string::npos) {
			value->clear();
			return false;
		}

		if (iter)
			*iter = reinterpret_cast<void*>(i + 1);
		value->assign(parsed_[i].value_begin, parsed_[i].value_end);
		return true;
	}

	bool HttpResponseHeaders::HasHeaderValue(const std::string& name, const std::string& value) const
	{
		void* iter = NULL;
		std::string temp;
		while (EnumerateHeader(&iter, name, &temp)) {
			if (value.size() == temp.size() && !_stricmp(temp.c_str(), value.c_str()))
				return true;
		}
		return false;
	}

	bool HttpResponseHeaders::HasHeader(const std::string& name) const
	{
		return FindHeader(0, name) != std::string::npos;
	}

	bool HttpResponseHeaders::IsRedirect(std::string* location) const
	{
		if (!IsRedirectResponseCode(response_code_))
			return false;

		// If we lack a Location header, then we can't treat this as a redirect.
		// We assume that the first non-empty location value is the target URL that
		// we want to follow.  TODO(darin): Is this consistent with other browsers?
		size_t i = std::string::npos;
		do {
			i = FindHeader(++i, "location");
			if (i == std::string::npos)
				return false;
			// If the location value is empty, then it doesn't count.
		} while (parsed_[i].value_begin == parsed_[i].value_end);

		if (location) {
			// Escape any non-ASCII characters to preserve them.  The server should
			// only be returning ASCII here, but for compat we need to do this.
			*location = EscapeNonASCII(
				std::string(parsed_[i].value_begin, parsed_[i].value_end));
		}

		return true;
	}

	bool HttpResponseHeaders::IsRedirectResponseCode(int response_code)
	{
		return (response_code == 301 ||
			response_code == 302 ||
			response_code == 303 ||
			response_code == 307 ||
			response_code == 308);
	}

	int64 HttpResponseHeaders::GetContentLength() const
	{
		return GetInt64HeaderValue("content-length");
	}

	int64 HttpResponseHeaders::GetInt64HeaderValue(const std::string& header) const
	{
		void* iter = NULL;
		std::string content_length_val;
		if (!EnumerateHeader(&iter, header, &content_length_val))
			return -1;

		if (content_length_val.empty())
			return -1;

		if (content_length_val[0] == '+')
			return -1;

		int64 result;
		bool ok = StringToInt64(content_length_val.begin(),
			content_length_val.end(), &result);
		if (!ok || result < 0)
			return -1;

		return result;
	}

	// From RFC 2616 14.16:
	// content-range-spec =
	//     bytes-unit SP byte-range-resp-spec "/" ( instance-length | "*" )
	// byte-range-resp-spec = (first-byte-pos "-" last-byte-pos) | "*"
	// instance-length = 1*DIGIT
	// bytes-unit = "bytes"
	bool HttpResponseHeaders::GetContentRange(int64* first_byte_position, 
		int64* last_byte_position, int64* instance_length) const
	{
		void* iter = NULL;
		std::string content_range_spec;
		*first_byte_position = *last_byte_position = *instance_length = -1;
		if (!EnumerateHeader(&iter, kContentRange, &content_range_spec))
			return false;

		// If the header value is empty, we have an invalid header.
		if (content_range_spec.empty())
			return false;

		size_t space_position = content_range_spec.find(' ');
		if (space_position == std::string::npos)
			return false;

		// Invalid header if it doesn't contain "bytes-unit".
		std::string::const_iterator content_range_spec_begin =
			content_range_spec.begin();
		std::string::const_iterator content_range_spec_end =
			content_range_spec.begin() + space_position;
		TrimLWS(&content_range_spec_begin, &content_range_spec_end);
		if (_strnicmp(&content_range_spec_begin[0], "bytes", 
			content_range_spec_end - content_range_spec_begin)) {
			return false;
		}

		size_t slash_position = content_range_spec.find('/', space_position + 1);
		if (slash_position == std::string::npos)
			return false;

		// Obtain the part behind the space and before slash.
		std::string::const_iterator byte_range_resp_spec_begin =
			content_range_spec.begin() + space_position + 1;
		std::string::const_iterator byte_range_resp_spec_end =
			content_range_spec.begin() + slash_position;
		TrimLWS(&byte_range_resp_spec_begin, &byte_range_resp_spec_end);

		// Parse the byte-range-resp-spec part.
		std::string byte_range_resp_spec(byte_range_resp_spec_begin,
			byte_range_resp_spec_end);
		// If byte-range-resp-spec != "*".
		if (byte_range_resp_spec != "*") {
			size_t minus_position = byte_range_resp_spec.find('-');
			if (minus_position != std::string::npos) {
				// Obtain first-byte-pos.
				std::string::const_iterator first_byte_pos_begin =
					byte_range_resp_spec.begin();
				std::string::const_iterator first_byte_pos_end =
					byte_range_resp_spec.begin() + minus_position;
				TrimLWS(&first_byte_pos_begin, &first_byte_pos_end);

				bool ok = StringToInt64(first_byte_pos_begin,
					first_byte_pos_end,
					first_byte_position);

				// Obtain last-byte-pos.
				std::string::const_iterator last_byte_pos_begin =
					byte_range_resp_spec.begin() + minus_position + 1;
				std::string::const_iterator last_byte_pos_end =
					byte_range_resp_spec.end();
				TrimLWS(&last_byte_pos_begin, &last_byte_pos_end);

				ok &= StringToInt64(last_byte_pos_begin,
					last_byte_pos_end,
					last_byte_position);
				if (!ok) {
					*first_byte_position = *last_byte_position = -1;
					return false;
				}
				if (*first_byte_position < 0 || *last_byte_position < 0 ||
					*first_byte_position > *last_byte_position)
					return false;
			}
			else {
				return false;
			}
		}

		// Parse the instance-length part.
		// If instance-length == "*".
		std::string::const_iterator instance_length_begin =
			content_range_spec.begin() + slash_position + 1;
		std::string::const_iterator instance_length_end =
			content_range_spec.end();
		TrimLWS(&instance_length_begin, &instance_length_end);

		if (*instance_length_begin == '*' && 
			instance_length_end - instance_length_begin == 1) {
			return false;
		}
		else if (!StringToInt64(instance_length_begin,
			instance_length_end,
			instance_length)) {
			*instance_length = -1;
			return false;
		}

		// We have all the values; let's verify that they make sense for a 206
		// response.
		if (*first_byte_position < 0 || *last_byte_position < 0 ||
			*instance_length < 0 || *instance_length - 1 < *last_byte_position)
			return false;

		return true;
	}

	void HttpResponseHeaders::Parse(const std::string& raw_input)
	{
		raw_headers_.reserve(raw_input.size());

		// ParseStatusLine adds a normalized status line to raw_headers_
		std::string::const_iterator line_begin = raw_input.begin();
		std::string::const_iterator line_end =
			std::find(line_begin, raw_input.end(), '\0');
		// has_headers = true, if there is any data following the status line.
		// Used by ParseStatusLine() to decide if a HTTP/0.9 is really a HTTP/1.0.
		bool has_headers = (line_end != raw_input.end() &&
			(line_end + 1) != raw_input.end() &&
			*(line_end + 1) != '\0');
		ParseStatusLine(line_begin, line_end, has_headers);
		raw_headers_.push_back('\0');  // Terminate status line with a null.

		if (line_end == raw_input.end()) {
			raw_headers_.push_back('\0');  // Ensure the headers end with a double null.

			assert('\0' == raw_headers_[raw_headers_.size() - 2]);
			assert('\0' == raw_headers_[raw_headers_.size() - 1]);
			return;
		}

		// Including a terminating null byte.
		size_t status_line_len = raw_headers_.size();

		// Now, we add the rest of the raw headers to raw_headers_, and begin parsing
		// it (to populate our parsed_ vector).
		raw_headers_.append(line_end + 1, raw_input.end());

		// Ensure the headers end with a double null.
		while (raw_headers_.size() < 2 ||
			raw_headers_[raw_headers_.size() - 2] != '\0' ||
			raw_headers_[raw_headers_.size() - 1] != '\0') {
			raw_headers_.push_back('\0');
		}

		// Adjust to point at the null byte following the status line
		line_end = raw_headers_.begin() + status_line_len - 1;

		{
			base::StringTokenizer lines(line_end + 1, raw_headers_.end(),
				std::string(1, '\0'));
			while (lines.GetNext()) {
				std::string::const_iterator name_begin = lines.token_begin();
				std::string::const_iterator values_end = lines.token_end();
				std::string::const_iterator values_begin;
				std::string::const_iterator name_end;

				std::string::const_iterator colon(std::find(name_begin, values_end, ':'));
				if (colon == values_end)
					continue;  // skip malformed header

				name_end = colon;

				// If the name starts with LWS, it is an invalid line.
				// Leading LWS implies a line continuation, and these should have
				// already been joined by AssembleRawHeaders().
				if (name_begin == name_end || IsLWS(*name_begin))
					continue;

				TrimLWS(&name_begin, &name_end);
				if (name_begin == name_end)
					continue;  // skip malformed header

				values_begin = colon + 1;
				TrimLWS(&values_begin, &values_end);

				// if we got a header name, then we are done.
				AddHeader(name_begin,
					name_end,
					values_begin,
					values_end);
			}
		}

		assert('\0' == raw_headers_[raw_headers_.size() - 2]);
		assert('\0' == raw_headers_[raw_headers_.size() - 1]);
	}

	void HttpResponseHeaders::ParseStatusLine(std::string::const_iterator line_begin, 
		std::string::const_iterator line_end, bool has_headers)
	{
		// TODO(eroman): this doesn't make sense if ParseVersion failed.
		std::string::const_iterator p = std::find(line_begin, line_end, ' ');

		if (p == line_end) {
			//DVLOG(1) << "missing response status; assuming 200 OK";
			raw_headers_.append(" 200 OK");
			response_code_ = 200;
			return;
		}

		// Skip whitespace.
		while (*p == ' ')
			++p;

		std::string::const_iterator code = p;
		while (*p >= '0' && *p <= '9')
			++p;

		if (p == code) {
			//DVLOG(1) << "missing response status number; assuming 200";
			raw_headers_.append(" 200 OK");
			response_code_ = 200;
			return;
		}
		raw_headers_.push_back(' ');
		raw_headers_.append(code, p);
		raw_headers_.push_back(' ');

		response_code_ = strtol(&code[0], 0, 10);
		//base::StringToInt(StringPiece(code, p), &response_code_);

		// Skip whitespace.
		while (*p == ' ')
			++p;

		// Trim trailing whitespace.
		while (line_end > p && line_end[-1] == ' ')
			--line_end;

		if (p == line_end) {
			//DVLOG(1) << "missing response status text; assuming OK";
			// Not super critical what we put here. Just use "OK"
			// even if it isn't descriptive of response_code_.
			raw_headers_.append("OK");
		}
		else {
			raw_headers_.append(p, line_end);
		}
	}

	size_t HttpResponseHeaders::FindHeader(size_t from, const std::string& name) const
	{
		for (size_t i = from; i < parsed_.size(); ++i) {
			if (parsed_[i].is_continuation())
				continue;
			const std::string::const_iterator& name_begin = parsed_[i].name_begin;
			const std::string::const_iterator& name_end = parsed_[i].name_end;
			if (static_cast<size_t>(name_end - name_begin) == name.size() &&
				!_strnicmp(&name_begin[0], name.c_str(), name.size()))
				return i;
		}

		return std::string::npos;
	}


	void HttpResponseHeaders::AddHeader(std::string::const_iterator name_begin, 
		std::string::const_iterator name_end, 
		std::string::const_iterator values_begin, 
		std::string::const_iterator values_end)
	{
		// If the header can be coalesced, then we should split it up.
		if (values_begin == values_end ||
			IsNonCoalescingHeader(name_begin, name_end)) {
			AddToParsed(name_begin, name_end, values_begin, values_end);
		}
		else 
		{
			base::StringTokenizer values(values_begin, values_end, std::string(1, ','));
			values.set_quote_chars("\'\"");
			while (values.GetNext()) {
				std::string::const_iterator value_begin = values.token_begin();
				std::string::const_iterator value_end = values.token_end();
				TrimLWS(&value_begin, &value_end);

				// bypass empty values.
				if (value_begin == value_end)
					continue;

				AddToParsed(name_begin, name_end, value_begin, value_end);
				name_begin = name_end = raw_headers_.end();
			}
		}
	}


	void HttpResponseHeaders::AddToParsed(std::string::const_iterator name_begin, std::string::const_iterator name_end, std::string::const_iterator value_begin, std::string::const_iterator value_end)
	{
		ParsedHeader header;
		header.name_begin = name_begin;
		header.name_end = name_end;
		header.value_begin = value_begin;
		header.value_end = value_end;
		parsed_.push_back(header);
	}

	void HttpResponseHeaders::AddRef()
	{
		InterlockedIncrement(&ref_count_);
	}

	void HttpResponseHeaders::Release()
	{
		if (InterlockedDecrement(&ref_count_) == 0)
		{
			delete this;
		}
	}

}