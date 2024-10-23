#include "FileState.h"


FileState::FileState(HWND hwnd)
	: hwnd(hwnd)
	, m_open_path{}
	, m_input_vec{}
{
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0'; // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L".txt";	// L"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

FileState::~FileState()
{
}


std::string FileState::get_path()
{
	// If user specifies filename and clicks OK, then return value is nonzero. The buffer pointed to by the lpstrFile member of the OPENFILENAME struct contains t he full path and file name specified.
	if (GetOpenFileName(&ofn) == TRUE)
	{
		hf = CreateFile(
			ofn.lpstrFile,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, // 0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL
		);
		std::cout << "FilePath successfully retrieved " << std::endl; //this->ofn.lpstrFile
		m_open_path = WideToStr(szFile);
		return m_open_path;
	}
	std::cerr << "Failed to retrieve FilePath" << std::endl;
	return m_open_path;
}

void FileState::read_file()
{

	std::ifstream ifs(m_open_path);  // Open the file
	
	std::string line;
	
	if (ifs.is_open()) {
		while (std::getline(ifs, line))
		{
			m_input_vec.push_back(line);
		}
		ifs.close();
	}
	else {
		std::cerr << "Unable to open file\n";
	}

}

std::string FileState::WideToStr(const std::wstring& wstr)
{
	// Uses windows function
	int size_in_bytes = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_in_bytes, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_in_bytes, NULL, NULL);
	return strTo;
}

std::string FileState::format_string(const std::string& open_path)
{
	std::string newPath = open_path;
	std::replace(newPath.begin(), newPath.end(), '\\', '/');
	return newPath;
}

