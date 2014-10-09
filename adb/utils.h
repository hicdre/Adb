#pragma once
#include <string>

std::wstring MultiByteToWide(const std::string& mb, UINT32 code_page = CP_UTF8);
std::string WideToMultiByte(const std::wstring& wide, UINT32 code_page = CP_UTF8);