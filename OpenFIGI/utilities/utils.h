#pragma once
#include <memory>
#include <string>
#include <Windows.h>

class Utils
{
public:
	
	// Singleton
	static Utils& GetInstance();

	std::string wideToStr(const std::wstring& wstr);
	std::wstring strToWide(const std::string& str);


private:

	Utils();

	static std::unique_ptr<Utils> m_pInstance;

};