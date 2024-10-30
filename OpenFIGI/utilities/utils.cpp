#include "utils.h"

std::unique_ptr<Utils> Utils::m_pInstance = nullptr;

Utils::Utils()
{

}


Utils& Utils::GetInstance()
{
    if (!m_pInstance)
    {
        m_pInstance.reset(new Utils());
    }
    return *m_pInstance;
}

// Converts a wide character string (std::wstring), typically in UTF-16 format, to a multi-byte string (std::string) in UTF-8 encoding.
std::string Utils::wideToStr(const std::wstring& wstr)
{
    // WideCharToMultiByte returns the number of bytes written to lpMultiByteStr.
    // Calculates the number of bytes needed for the conversion from wstr to a mutli-byte character string, std::string
    int byte_count = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(byte_count, 0); // Init string with byte_count characters all set to 0.
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], byte_count, NULL, NULL);
    return str;
}

// Converts a multi-byte string (std::string) in UTF-8 format to a wide character string (std::wstring) in UTF-16 encoding.
std::wstring Utils::strToWide(const std::string& str)
{
    // MultiByteToWideChar returns the number of wide characters written to lpWideCharStr.
    // Determines the number of wide characters needed for conversion.
    int wide_char_count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(wide_char_count, 0);  // Init wstring with wide_char_count characters all set to 0.
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], wide_char_count);
    return wstr;
}
