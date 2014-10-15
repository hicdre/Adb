#pragma once
#include <functional>

namespace net
{
	class HttpResponseStringWriter;
	class HttpResponseWriter
	{
	public:
		~HttpResponseWriter() {}

		virtual void Init() = 0;

		virtual void Write(const char* buffer, int num_bytes) = 0;

		virtual void Finish() = 0;

		virtual HttpResponseStringWriter* GetAsStringWriter() = 0;
	};

	class HttpResponseStringWriter : public HttpResponseWriter
	{
	public:
		HttpResponseStringWriter();
		~HttpResponseStringWriter();

		virtual void Init() override;
		virtual void Write(const char* buffer, int num_bytes) override;
		virtual void Finish() override;
		virtual HttpResponseStringWriter* GetAsStringWriter() override;

		const std::string& GetString() const;
	private:
		std::string contents_;
	};
}