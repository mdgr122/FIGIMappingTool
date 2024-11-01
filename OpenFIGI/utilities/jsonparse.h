#pragma once
#include <json.hpp>
#include <fstream>
#include <iostream>

class JsonParse
{
public:
	JsonParse();
	~JsonParse();

	void read_json(nlohmann::json response);

	std::vector<std::string> get_vec();

private:
	nlohmann::json m_json_data;
	std::vector<std::string> m_csv_vec{};
};