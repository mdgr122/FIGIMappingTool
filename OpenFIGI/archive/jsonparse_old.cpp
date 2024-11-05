#include "jsonparse.h"
#include <cassert>

JsonParse::JsonParse() : m_json_data{}, m_csv_vec{}
{
}

JsonParse::~JsonParse()
{
}


/*
	Your JsonParse class is designed to read JSON data and convert it into a CSV format, storing each line of the CSV in a vector of strings (m_csv_vec). The read_json function processes the JSON data and populates m_csv_vec.

	However, the current implementation has several issues:

		Complex and Nested Loops: The code uses deeply nested loops with multiple counters and unclear variable names, making it hard to read and maintain.
		Inefficient Handling of JSON Data: There's redundant access to JSON elements, and the code doesn't efficiently iterate over the JSON structure.
		Lack of Error Handling: The code doesn't handle potential errors, such as invalid JSON structures or missing keys.
		Unclear Variable Names: Variables like t, i, el, nel, counter, etc., make the code less readable.
		Magic Numbers and Conditions: Conditions like row_line.size() > 99 are unexplained, and the purpose is unclear.
		No Use of Modern C++ Features: The code doesn't utilize features like range-based loops, auto type deduction, or standard algorithms that can simplify the code.


	Goals for Improvement

		Enhance Readability: Use meaningful variable names and simplify the logic.
		Improve Efficiency: Optimize the way JSON data is accessed and processed.
		Add Error Handling: Ensure the code gracefully handles unexpected JSON structures.
		Utilize Modern C++ Features: Make use of language features that simplify the code.
		Maintainability: Structure the code so it's easier to maintain and extend in the future.
*/

void JsonParse::read_json(const nlohmann::json& response)
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
	//const nlohmann::json& first_obj = m_json_data.at(1);


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

	//int t{};
	//nlohmann::json json_obj = m_json_data[t];
	//nlohmann::json json_data_obj = m_json_data[t]["data"];

	//size_t main_counter = 0;
	//size_t counter = 0;

	//std::string header_line{};
	//std::string row_line{};
	//
	//// Header Loop
	//// Loops through each object in the array.
	//for (nlohmann::json::iterator i = json_obj.begin(); i != json_obj.end(); ++i)
	//{
	//	//vec.push_back(i.key());
	//	if (i.key() != "data")
	//	{
	//		header_line.append(i.key());
	//		header_line.append(",");
	//	}


	//	if (i.key() == "data")
	//	{
	//		// Loops through each object in a json object whose value is an array of objects
	//		for (nlohmann::json::iterator el = (i.value()).begin(); el != (i.value()).end(); ++el)
	//		{

	//			while (counter < json_obj["data"][0].size())
	//			{
	//				for (nlohmann::json::iterator nel = (el.value()).begin(); nel != (el.value()).end(); ++nel)
	//				{

	//					header_line.append(nel.key());
	//					header_line.append(",");
	//					counter++;
	//				}
	//			}
	//		}
	//	}


	//}
	//m_csv_vec.push_back(header_line);


	//size_t json_obj_count = 0;
	//size_t data_obj_count = 0;
	//size_t parent_counter = 0;
	//size_t child_counter = 1;
	//size_t child_child_counter = 0;
	//// 1

	//nlohmann::json data = m_json_data;


	//for (nlohmann::json::iterator it = data.begin(); it != data.end(); ++it)
	//{
	//	nlohmann::json json_obj = m_json_data[t];
	//	for (nlohmann::json::iterator i = json_obj.begin(); i != json_obj.end(); ++i)
	//	{

	//		// 2
	//		// Loops through each object in a json object whose value is an array of objects
	//		for (nlohmann::json::iterator el = (i.value()).begin(); el != (i.value()).end(); ++el)
	//		{
	//			child_counter = 1;
	//			child_child_counter = 0;
	//			if (!row_line.empty())
	//			{
	//				if (row_line.size() > 99)
	//				{
	//					m_csv_vec.push_back(row_line);

	//				}
	//			}
	//			row_line.clear();

	//			for (nlohmann::json::iterator nel = (el.value()).begin(); nel != (el.value()).end(); ++nel)
	//			{
	//				if (i.key() == "data")
	//				{
	//					if (!nel.value().empty())
	//					{
	//						row_line.append(nel.value());
	//					}
	//					else
	//					{
	//						row_line.append("");
	//					}
	//					row_line.append(",");
	//					child_counter++;
	//				}
	//			}
	//			//std::cout << "Row Line: " << row_line << std::endl;

	//			//std::cout << json_obj.size() << std::endl;
	//			while (child_child_counter <= json_obj.size())
	//			{
	//				// for each obj in data, 2, loop through for the amount of objs in json_obj (i.e., loop through 4 objects twice)
	//				for (nlohmann::json::iterator j = json_obj.begin(); j != json_obj.end(); j++)
	//				{
	//					if (!(j.value().is_array()))
	//					{
	//						if (j.value() == nullptr)
	//						{
	//							row_line.append("");
	//						}
	//						else
	//						{
	//							row_line.append(j.value());
	//						}
	//						row_line.append(",");
	//					}
	//					child_child_counter++;

	//				}
	//			}


	//		}

	//		parent_counter++;
	//	}
	//	t++;
	//}


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
		if (main_values.count(key) && key_count <= main_values.size())
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
