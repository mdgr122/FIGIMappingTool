#pragma once
#include <json.hpp>
#include <fstream>
#include <iostream>

class JsonParse
{
public:
	JsonParse();
	~JsonParse();

	void read_json();

private:
	nlohmann::json m_json_data;
};