/*
 * This file is part of FIGIMappingTool.
 *
 * Copyright 2023 Michael Dakin-Green
 *
 * Portions Copyright 2017 Bloomberg Finance L.P.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Request.h"

using namespace nlohmann;

Request::Request(FileState& fileState)
	: m_fileState(fileState)
	, r{}
	, m_IdentifierPairs{}
	, m_IdentifierType{ IdentifierType::NONE }
	, m_response{ json::array() }
	, m_bad_response{ json::array() }
	, m_current_request_valid{ json::array() }
	, m_all_requests_valid{}
	, m_combined{json::array()}
{

}

void Request::GetVec()
{
	m_Identifiers = m_fileState.GetVec();
}

// A vector of pairs, where each pair contains a string identifier (T1) and an identifierType (T2).
// A pair is a specific case of a std::tulipe with two elements.
// If neither T1 nor T2 is a possibly cv-qualified class type with non-trivial destructor, or array thereof, the destructor of pair is trivial.
void Request::GetIdentifiers()
{	
	size_t counter = 0;
	size_t job_count = 0;
	
	json jsonBody;


	Timer timer;
	timer.Start(); // begin the timer
	bool api_cooldown;
	
	// 100 MAPPING REQUESTS	
	for (auto& elem : m_IdentifierPairs)
	{
		counter++;
		
		std::string& idValue = elem.first;
		std::string& id_context_value = elem.second;
		std::string idType = validate_base_identifier(idValue);
		std::string id_context_type = validate_context_identifier(id_context_value);

		//std::cout << "{\"idValue\": " << idValue << ", \"idType\": " << idType << ", \"idContext\": " << id_context_value << "}" << std::endl;



		if (idType != "NONE")
		{
			if (!id_context_type.empty())
			{
				if (idType == "TICKER")
				{
					process_ticker(idValue);
				}

				jsonBody =
				{
					{"idType", idType},
					{"idValue", idValue},
					{id_context_type, id_context_value},
					{"includeUnlistedEquities", true}
				};

			}
			else
			{
				if (idType == "TICKER")
				{
					process_ticker(idValue);
				}

				jsonBody =
				{
					{"idType", idType},
					{"idValue", idValue},
					{"includeUnlistedEquities", true}
				};

			}
			m_current_request_valid.push_back(jsonBody);
			m_all_requests_valid.push_back(jsonBody);
		}
		else
		{
			jsonBody =
			{
				{"idType", idType},
				{"idValue", idValue},
				{ "warning", "Skipped - Invalid idType" },
			};


			m_bad_response.push_back(jsonBody);

		}


		if (job_count >= 26 && timer.ElapsedSec() <= 6)
		{
			api_cooldown = true;
			while (api_cooldown)
			{
				// Sleep for a short duration to prevent busy-waiting
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				if (timer.ElapsedSec() > 6)
				{
					job_count = 0;
					timer.Stop();
					timer.Start();
					api_cooldown = false;
				}
			}
		}


		if (m_current_request_valid.size() == 100 || (m_IdentifierPairs.size() - counter) == 0)
		{

			r = cpr::Post(
				cpr::Url{ "https://api.FIGIMappingTool.com/v3/mapping" },
				cpr::Header{
					{"Content-Type", "application/json"},
					{"X-FIGIMappingTool-APIKEY", m_apikey}
				},
				cpr::Body{ m_current_request_valid.dump() }
			);
			if (r.status_code != 200)
			{
				std::cout << "Error " << r.status_code << " | " << r.text << std::endl;
				return;
			}
			job_count++;

			nlohmann::json responseJson = json::parse(r.text);

			m_response.insert(m_response.end(), responseJson.begin(), responseJson.end());
			m_current_request_valid.clear(); // Clears the current requests.

		}
	}


}

void Request::ParseResponse()
{
	size_t counter = 0;
	m_combined.clear(); // Clear m_combined to ensure it's empty before storing new data
	//std::cout << "m_all_requests_valid.size(): " << m_all_requests_valid.size() << std::endl;
	//std::cout << "m_response.size(): " << m_response.size() << std::endl;

	for (const auto& elem : m_response)
	{
		json updated_elem = elem;

		for (auto it = m_all_requests_valid[counter].begin(); it != m_all_requests_valid[counter].end(); ++it)
		{
			if (it.key() != "includeUnlistedEquities")
			{
				updated_elem[it.key()] = it.value();
			}
		}
		m_combined.push_back(updated_elem); // Store the modified element in m_combined
		counter++;
	}

	for (const auto& elem_2 : m_bad_response)
	{
		m_combined.push_back(elem_2);
	}
}


std::string Request::validate_base_identifier(const std::string& identifier)
{
	std::string idType = "";
	const std::regex ticker_pattern("^(?!BBG[0-9A-Z]{9}$)([A-Za-z0-9]+)([-./]?[A-Za-z]{1,2})?( Index)?$");
	const std::regex isin_pattern("^(?!BBG)[A-Z]{2}[A-Z0-9]{9}[0-9]$");
	const std::regex sedol_pattern("^[0-9B-DF-HJ-NP-TV-Z]{6}[0-9]$");
	const std::regex cusip_pattern("^[0-9]{3}[0-9A-Z]{6}$");
	const std::regex figi_pattern("^BBG[0-9A-Z]{9}$");


	if (!identifier.empty())
	{
		if (std::regex_match(identifier, isin_pattern))
		{
			idType = "ID_ISIN";
		}
		else if (std::regex_match(identifier, sedol_pattern))
		{
			idType = "ID_SEDOL";
		}
		else if (std::regex_match(identifier, cusip_pattern))
		{
			idType = "ID_CUSIP";
		}
		else if (std::regex_match(identifier, ticker_pattern))
		{
			idType = "TICKER";
		}
		else if (std::regex_match(identifier, figi_pattern))
		{
			idType = "COMPOSITE_ID_BB_GLOBAL";
		}
		else
		{
			idType = "NONE";
		}
		return idType;

		/*
		---Removed---
		const std::regex wkn_pattern("^[A-HJ-NP-Z0-9]{6}$");
		const std::regex ticker_pattern("([A-Za-z]{1,5})([-./]?[A-Za-z]{1,2})?");
		const std::regex ticker_pattern("^([A-Za-z0-9]+)([-./]?[A-Za-z]{1,2})?( Index)?$");
		const std::regex cusip_pattern("^[0-9]{3}[0-9a-zA-Z]{5}[0-9]");
		idType = "ID_BB_GLOBAL";
		else if (std::regex_match(identifier, wkn_pattern))
		{
			idType = "ID_WERTPAPIER";
		}
		*/
	}

	return idType;
}

std::string Request::validate_context_identifier(const std::string& context_identifier)
{
	std::string context_property = "";
	
	if (context_identifier.length() == 2)
	{
		context_property = "exchCode";
	}
	else if (context_identifier.length() == 3)
	{
		context_property = "currency";
	}
	else if (context_identifier.length() == 4)
	{
		context_property = "micCode";
	}
	else
	{
		context_property = "";
	}
	return context_property;

}

void Request::process_ticker(std::string& str)
{
	size_t index_check = str.find(" Index"); // Checks if substring " Index" exists in str.
	std::string ticker_str = str;
	std::string index_str = "";

	if (index_check != std::string::npos) // If " Index" is found, returns the position.
	{
		ticker_str = str.substr(0, index_check);
		index_str = str.substr(index_check);
		//return; // Returns, leaving the ticker as "Ticker Index"
	}

	// Process ticker_str only
	size_t dotPos = ticker_str.find('.');
	if (dotPos != std::string::npos) 
	{
		size_t afterDot = dotPos + 1;
		size_t charsAfterDot = ticker_str.size() - afterDot;

		// if '.' at the end, remove it
		if (charsAfterDot == 0) 
		{
			ticker_str.erase(dotPos, 1);
		}
		else if (charsAfterDot == 1) 
		{
			// if one character after '.', replace '.' with '/'
			ticker_str[dotPos] = '/';
		}
		else if (charsAfterDot > 1) 
		{
			// if multiple characters after '.', replace '.' with '-', keep only immediate character
			ticker_str.replace(dotPos, charsAfterDot + 1, "-" + ticker_str.substr(afterDot, 1));
		}
	}
	str = ticker_str + index_str;
}

bool Request::PeakResponse()
{
	if (!m_response.empty())
	{
		return true;
	}
	return false;
}

nlohmann::json Request::GetResponse()
{
	if (!m_response.empty())
	{
		ParseResponse();
		return m_combined;
	}
	return nullptr;
}

void Request::ClearResponse()
{
	//m_Identifiers.clear();
	m_combined.clear();
	m_IdentifierPairs.clear();
	m_response.clear();
	m_current_request_valid.clear();
	m_all_requests_valid.clear();
}


std::string const Request::get_apikey()
{
	return m_apikey;
}

void Request::set_apikey(std::wstring api_key)
{
	m_apikey = Utils::GetInstance().wideToStr(api_key);
}


std::vector<std::pair<std::string, std::string>> Request::GetIdentifierType()
{
	// Loop through each element in m_Identifiers, which is a row
	for (const auto& elem : m_Identifiers)
	{
		std::istringstream iss(elem);
		std::string base_identifier = "";
		std::string contextual_identifier = "";

		// Read the base identifier
		if (iss >> base_identifier)
		{
			// Attempt to read the contextual identifier
			if (iss >> contextual_identifier)
			{
				// If the contextual identifier is "Index", append it to base_identifier
				if (contextual_identifier == "Index")
				{
					base_identifier += " Index";
					contextual_identifier = ""; // Clear contextual identifier
				}
			}
			else
			{
				// If there's no contextual identifier, ensure it's empty
				contextual_identifier = "";
			}

			// Add the pair to the vector (only once)
			m_IdentifierPairs.push_back(std::make_pair(base_identifier, contextual_identifier));
		}
	}

	return m_IdentifierPairs;
}