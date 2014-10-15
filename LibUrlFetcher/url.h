#pragma once

namespace net
{
	class URL
	{
	public:
		URL();
		URL(const std::string& url);
		~URL();

		bool IsValid() const { return valid_; }

		std::string host() const { return host_; }
		std::string path() const { return path_; }

		void set_path(std::string path);

		std::string to_string() const;
	private:
		void ParseURL(const std::string& url);
		bool valid_;
		std::string host_;
		std::string path_;
	};

	std::string EscapeNonASCII(const std::string& input);
}