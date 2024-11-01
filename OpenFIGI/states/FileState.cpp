#include "FileState.h"


//FileState::FileState(HWND hwnd)
FileState::FileState()
	: hwnd(nullptr)
	, hf(nullptr)
	, m_open_path{}
	, m_save_path{}
	, m_input_vec{}
	, ofn_open{}
	, ofn_save{}
{
}

FileState::~FileState()
{
}


std::string FileState::get_open_path()
{
	ZeroMemory(&ofn_open, sizeof(ofn_open));
	ofn_open.lStructSize = sizeof(ofn_open);
	ofn_open.hwndOwner = hwnd;
	ofn_open.lpstrFile = szFile;
	ofn_open.lpstrFile[0] = '\0'; // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn_open.nMaxFile = sizeof(szFile);
	ofn_open.lpstrFilter = L"Text Files (*.txt)\0*.txt\0CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";	//L"All\0*.*\0Text\0*.TXT\0";
	ofn_open.nFilterIndex = 1;
	ofn_open.lpstrFileTitle = NULL;
	ofn_open.nMaxFileTitle = 0;
	ofn_open.lpstrInitialDir = NULL;
	ofn_open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// If user specifies filename and clicks OK, then return value is nonzero. The buffer pointed to by the lpstrFile member of the OPENFILENAME struct contains t he full path and file name specified.
	if (GetOpenFileName(&ofn_open) == TRUE)
	{
		hf = CreateFile(
			ofn_open.lpstrFile,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, // 0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL
		);
		std::cout << "FilePath successfully retrieved " << std::endl; //this->ofn.lpstrFile
		m_open_path = Utils::GetInstance().wideToStr(szFile);
		return m_open_path;
	}
	std::cerr << "Failed to retrieve FilePath" << std::endl;
	return m_open_path;
}

std::string FileState::get_save_path()
{
	ZeroMemory(&ofn_save, sizeof(ofn_save));
	ofn_save.lStructSize = sizeof(ofn_save);
	ofn_save.hwndOwner = hwnd;
	ofn_save.lpstrFile = szFile;
	ofn_save.lpstrFile[0] = '\0'; // Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn_save.nMaxFile = sizeof(szFile);
	ofn_save.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0JSON Files (*.json)\0*.json\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0"; //L".json;.txt";
	ofn_save.nFilterIndex = 1;
	ofn_save.lpstrFileTitle = NULL;
	ofn_save.nMaxFileTitle = 0;
	ofn_save.lpstrInitialDir = NULL;
	ofn_save.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// If user specifies filename and clicks OK, then return value is nonzero. The buffer pointed to by the lpstrFile member of the OPENFILENAME struct contains t he full path and file name specified.
	if (GetSaveFileName(&ofn_save) == TRUE)
	{
		hf = CreateFile(
			ofn_save.lpstrFile,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, // 0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL
		);
		std::cout << "FilePath successfully retrieved " << std::endl; //this->ofn.lpstrFile
		m_save_path = Utils::GetInstance().wideToStr(szFile);
		return m_save_path;
	}
	std::cerr << "Failed to retrieve FilePath" << std::endl;
	return m_save_path;
}

void FileState::read_file(std::string file_path)
{
	if (file_path == m_open_path)
	{
		std::ifstream ifs(m_open_path);  // Open the file

		std::string line;

		if (ifs.is_open())
		{
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
	else
	{
		std::ifstream ifs(file_path);  // Open the file

		std::string line;

		if (ifs.is_open())
		{
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

}

void FileState::save_file(nlohmann::json response, std::string save_path)
{
	//std::string save_path = m_save_path;
	std::ofstream ofs(save_path);

	ofs << response.dump(4);

}

void FileState::save_csv_file(std::vector<std::string> vec, std::string save_path)
{
	std::ofstream ofs(save_path);



	for (const auto& elem : vec)
	{
		std::cout << elem << std::endl;
		ofs << elem << '\n';
	}
}

std::string FileState::format_string(const std::string& open_path)
{
	std::string newPath = open_path;
	std::replace(newPath.begin(), newPath.end(), '\\', '/');
	return newPath;
}

const HWND FileState::GetHWND() const
{
	HWND handle;
	handle = hwnd_ptr->GetHWND();
	if (handle != nullptr)
		return handle;
}

const std::vector<std::string>& FileState::GetVec() const
{
	return m_input_vec;
}

