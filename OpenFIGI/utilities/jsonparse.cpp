#include "jsonparse.h"
#include <cassert>

JsonParse::JsonParse() : m_json_data{}, m_csv_vec{}
{
}

JsonParse::~JsonParse()
{
}

void JsonParse::read_json(const nlohmann::json &response)
{
	m_csv_vec.clear();
	m_json_data = response;


	if (!m_json_data.is_array() || m_json_data.empty())
	{
		// TODO: Handling if m_json_data is not an array or if m_json_data is empty.
		return;
	}

	std::vector<std::string> header_keys;
	nlohmann::json first_obj;


	if (m_json_data.at(0).contains("data"))
	{
		first_obj = m_json_data.at(0);
	}
	else
	{
		int data_index = 0;
		for (const auto& item : m_json_data)
		{
			if (item.is_object() && item.contains("data"))
			{
				first_obj = m_json_data.at(data_index);
				break;
			}
			++data_index;
		}
	}



	for (const auto& [key, value] : first_obj.items())
	{
		// Collecting keys of objects of first object in the parent array, if key is not "data"
		if (key != "data" && key != "warning")
		{
			header_keys.push_back(key);
		}
	}

	// Now can handle the data key
	if (first_obj.contains("data") && first_obj["data"].is_array() && !first_obj["data"].empty())
	{
		const nlohmann::json& first_data_obj = first_obj["data"].at(0);
		for (const auto& [key, value] : first_data_obj.items())
		{
			header_keys.push_back(key);
		}
	}

	// Constructing the header
	std::string header_line = join_keys(header_keys);
	m_csv_vec.push_back(header_line);

	// Processing the actual response
	for (const auto& obj : m_json_data)
	{
		// Extracting the dict values, excluding the 'data' key
		std::map<std::string, std::string> main_values;
		for (const auto& [key, value] : obj.items())
		{
			if (key != "data")
			{
				main_values[key] = value_to_string(value);
			}
		}

		// Processing the "data" array if it exists
		if (obj.contains("data") && obj["data"].is_array())
		{
			for (const auto& data_item : obj["data"])
			{
				// Extracting object values from data array
				std::map<std::string, std::string> data_values;
				for (const auto& [key, value] : data_item.items())
				{
					data_values[key] = value_to_string(value);
				}

				// Building the rows
				std::string row_line = build_csv_row(header_keys, main_values, data_values);
				m_csv_vec.push_back(row_line);
			}
		}
		else
		{
			// Build row without "data" array
			std::string row_line = build_csv_row(header_keys, main_values, {});
			m_csv_vec.push_back(row_line);
		}
	}


}

std::string JsonParse::join_keys(const std::vector<std::string>& keys) const
{
	std::string line;
	for (const auto& key : keys)
	{
		line += key + ",";
	}
	if (!line.empty())
	{
		line.pop_back(); // removes trailing comma
	}
	return line;
}

std::string JsonParse::value_to_string(const nlohmann::json& value) const
{
	if (value.is_string())
	{
		return value.get<std::string>();
	}
	else if (value.is_null())
	{
		return "";
	}
	else
	{
		return value.dump();
	}
}

std::string JsonParse::build_csv_row(const std::vector<std::string>& header_keys, const std::map<std::string, std::string>& main_values, const std::map<std::string, std::string>& data_values) const
{
	std::string row_line;
	int key_count = 0;
	for (const auto& key : header_keys)
	{
		if (main_values.count(key) && key_count <= main_values.size()) // Added the key_count check because if there are duplicate keys in the request and response, its set to take value of request first if this check is not in place.
		{
			row_line += main_values.at(key);
		}
		else if (data_values.count(key))
		{
			row_line += data_values.at(key);
		}
		else if (data_values.count(key) == 0)
		{
			row_line += main_values.at("warning");
		}
		row_line += ",";
		++key_count;
	}
	if (!row_line.empty())
	{
		row_line.pop_back(); // Removes trailing comma
	}
	return row_line;
}

std::vector<std::string> JsonParse::get_vec()
{
	return m_csv_vec;
}
