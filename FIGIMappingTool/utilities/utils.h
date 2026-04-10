#pragma once
#include <string>
#include <Windows.h>

namespace Utils {

// Converts UTF-16 (std::wstring) to UTF-8 (std::string).
inline std::string wide_to_str(const std::wstring& wstr) {
  if (wstr.empty()) 
  {
    return {};
  }
  int byte_count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
  std::string str(byte_count, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), str.data(), byte_count, nullptr, nullptr);
  return str;
}

// Converts UTF-8 (std::string) to UTF-16 (std::wstring).
inline std::wstring str_to_wide(const std::string& str) {
  if (str.empty()) 
  {
    return {};
  }
  int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
  std::wstring wstr(count - 1, L'\0'); // -1: MultiByteToWideChar includes null terminator
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), count);
  return wstr;
}

} // namespace Utils
