#pragma once

#include <Windows.h>
#include <commdlg.h>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <json.hpp>

class FileState {
public:
  FileState() = default;
  ~FileState() = default;

  // Non-copyable (owns dialog state).
  FileState(const FileState&) = delete;
  FileState& operator=(const FileState&) = delete;

  std::string get_open_path();
  std::string get_save_path();

  void read_file(std::string_view file_path);
  void clear_data();

  // Save raw string content to a file (CSV or JSON text).
  void save_text_file(std::string_view content, std::string_view save_path);
  void save_json_file(const nlohmann::json& json, std::string_view save_path);

  [[nodiscard]] const std::vector<std::string>& get_lines() const { return m_input_vec; }

private:
  wchar_t m_szFile[260]{};
  std::vector<std::string> m_input_vec;
};
