#include "utils.h"

std::wstring MultiByteToWide(const std::string& mb, UINT32 code_page /*= CP_UTF8*/)
{
	if (mb.empty())
		return std::wstring();

	int mb_length = static_cast<int>(mb.length());
	// Compute the length of the buffer.
	int charcount = MultiByteToWideChar(code_page, 0,
		mb.data(), mb_length, NULL, 0);
	if (charcount == 0)
		return std::wstring();

	std::wstring wide;
	wide.resize(charcount);
	MultiByteToWideChar(code_page, 0, mb.data(), mb_length, &wide[0], charcount);

	return wide;
}

std::string WideToMultiByte(const std::wstring& wide, UINT32 code_page /*= CP_UTF8*/)
{
	int wide_length = static_cast<int>(wide.length());
	if (wide_length == 0)
		return std::string();

	// Compute the length of the buffer we'll need.
	int charcount = WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
		NULL, 0, NULL, NULL);
	if (charcount == 0)
		return std::string();

	std::string mb;
	mb.resize(charcount);
	WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
		&mb[0], charcount, NULL, NULL);

	return mb;
}
