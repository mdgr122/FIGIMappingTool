#pragma once
#include <json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>

class JsonParse
{
public:
	JsonParse();
	~JsonParse();

	void read_json(const nlohmann::json& response);
	std::string join_keys(const std::vector<std::string>& keys) const;
	std::string value_to_string(const nlohmann::json& value) const;
	std::string build_csv_row(const std::vector<std::string>& header_keys, const std::map<std::string, std::string>& main_values, const std::map<std::string, std::string>& data_values) const;

	std::vector<std::string> get_vec();

private:
	nlohmann::json m_json_data;
	std::vector<std::string> m_csv_vec{};
};