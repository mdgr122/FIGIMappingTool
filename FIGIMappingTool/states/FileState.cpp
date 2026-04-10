#include "FileState.h"
#include "../utilities/Utils.h"

std::string FileState::get_open_path()
{
    OPENFILENAME ofn{};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize    = sizeof(ofn);
    ofn.hwndOwner      = nullptr;
    ofn.lpstrFile      = m_szFile;
    ofn.lpstrFile[0]   = L'\0';
    ofn.nMaxFile       = sizeof(m_szFile) / sizeof(m_szFile[0]);
    ofn.lpstrFilter    = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex   = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle  = 0;
    ofn.lpstrInitialDir= nullptr;
    ofn.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        return Utils::wide_to_str(m_szFile);
    }
    return {};
}

std::string FileState::get_save_path()
{
    OPENFILENAME ofn{};
    ZeroMemory(&ofn, sizeof(ofn));

    std::wstring placeholder = L"response.csv";
    wcsncpy_s(m_szFile, placeholder.c_str(), placeholder.size() + 1);

    ofn.lStructSize    = sizeof(ofn);
    ofn.hwndOwner      = nullptr;
    ofn.lpstrFile      = m_szFile;
    ofn.nMaxFile       = sizeof(m_szFile) / sizeof(m_szFile[0]);
    ofn.lpstrFilter    = L"Compatible Files (*.txt;*.csv;*.json)\0*.txt;*.csv;*.json\0"
                         L"CSV Files (*.csv)\0*.csv\0"
                         L"JSON Files (*.json)\0*.json\0"
                         L"Text Files (*.txt)\0*.txt\0"
                         L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex   = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle  = 0;
    ofn.lpstrInitialDir= nullptr;
    ofn.Flags          = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        return Utils::wide_to_str(m_szFile);
    }
    return {};
}

void FileState::read_file(std::string_view file_path)
{
std::ifstream ifs{std::string{file_path}};
    
    if (!ifs.is_open()) return;

    std::string line;
    while (std::getline(ifs, line))
    {
        // Strip trailing \r if present (Windows line endings).
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (!line.empty())
        {
            m_input_vec.push_back(std::move(line));
        }
    }
}

void FileState::clear_data()
{
    m_input_vec.clear();
}

void FileState::save_text_file(std::string_view content, std::string_view save_path)
{
    std::ofstream ofs{std::string{save_path}};
    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
}

void FileState::save_json_file(const nlohmann::json& json, std::string_view save_path)
{
    std::ofstream ofs{std::string{save_path}};
    std::string dumped = json.dump(4);
    ofs.write(dumped.data(), static_cast<std::streamsize>(dumped.size()));
}