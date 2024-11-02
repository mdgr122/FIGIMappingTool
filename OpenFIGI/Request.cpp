#include "Request.h"
#include <cpr/cpr.h>
#include <json.hpp>
#include <regex>
#include <algorithm>

using namespace nlohmann;

//Request::Request()
//{
//}

Request::Request(FileState& fileState)
	: m_fileState(fileState)
	, m_IdentifierPairs{}
	, m_IdentifierType{ IdentifierType::NONE }
	, m_sResponse{json::array()}
	, m_RequestBody{json::array()}
	, m_AllRequestBody{}
{
	//GetVec(m_fileState);
	//GetIdentifierType();
	//GetIdentifiers();
}

// A vector of pairs, where each pair contains a string identifier (T1) and an identifierType (T2).
// A pair is a specific case of a std::tulipe with two elements.
// If neither T1 nor T2 is a possibly cv-qualified class type with non-trivial destructor, or array thereof, the destructor of pair is trivial.
void Request::GetIdentifiers()
{
	std::string idType;
	std::string exch_code;
	
	
	size_t counter = 0;
	size_t job_count = 0;
	
	json jsonBody;
	json requestBody = json::array();

	//json jsonArray;
	json badResponse = json::array({"warning", "No identifier found."});

	Timer timer;
	timer.Start(); // begin the timer
	bool api_cooldown;
	
	// 100 MAPPING REQUESTS	
	for (const auto& elem : m_IdentifierPairs)
	{
		counter++;
		std::string test = elem.first;
		switch (elem.second)
		{
		case Request::IdentifierType::ID_ISIN:
			idType = "ID_ISIN";
			break;
		case Request::IdentifierType::TICKER:
			idType = "TICKER";
			break;
		case Request::IdentifierType::ID_CUSIP:
			idType = "ID_CUSIP";
			break;
		case Request::IdentifierType::COMPOSITE_ID_BB_GLOBAL:
			idType = "COMPOSITE_ID_BB_GLOBAL";
			break;
		case Request::IdentifierType::ID_SEDOL:
			idType = "ID_SEDOL";
			break;
		case Request::IdentifierType::NONE:
			idType = "NONE";
			break;
		default:
				break;
		}
		if (idType == "ID_ISIN")
		{
			// Temporarily removing to allow all results
			//exch_code = ((elem.first.substr(0, 2)) == "US") ? "US" : "";
			exch_code = "";
			
			if (!exch_code.empty())
			{
				jsonBody =
				{
					{"idType", idType},
					{"idValue", elem.first},
					{"exchCode", exch_code},
					{"includeUnlistedEquities", true},
				};
			}
			else 
			{
				jsonBody =
				{
					{"idType", idType},
					{"idValue", elem.first},
					{"includeUnlistedEquities", true},
				};
			}
			requestBody.push_back(jsonBody);
			m_AllRequestBody.push_back(jsonBody);
		}
		else if(idType != "NONE")
		{
			jsonBody = 
			{
				{"idType", idType},
				{"idValue", elem.first},
				{"includeUnlistedEquities", true}
			};
			requestBody.push_back(jsonBody);
			m_AllRequestBody.push_back(jsonBody);
		}


		while (job_count >= 26 && timer.ElapsedSec() <= 6 && api_cooldown == true)
		{
			if (timer.ElapsedSec() > 6)
			{
				job_count = 0;
				timer.Stop();
				timer.Start();
				api_cooldown = false;
			}
		}

		if (requestBody.size() == 100 || (m_IdentifierPairs.size() - counter) == 0)
		{
			cpr::Response r = cpr::Post(
				cpr::Url{ "https://api.openfigi.com/v3/mapping" },
				cpr::Header{
					{"Content-Type", "application/json"},
					{"X-OPENFIGI-APIKEY", API_KEY}
				},
				cpr::Body{ requestBody.dump() }
			);
			job_count++;

			json responseJson = json::parse(r.text);
			m_sResponse.insert(m_sResponse.end(), responseJson.begin(), responseJson.end());
			m_RequestBody.insert(m_RequestBody.end(), responseJson.begin(), responseJson.end());
			requestBody.clear();

		}
	}
}


bool Request::Validate_ISIN(std::string& identifier)
{
	// ISIN REGEX
	const std::regex pattern("^(?!BBG)[A-Z]{2}[A-Z0-9]{9}[0-9]$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}
	
	return false;
}

bool Request::Validate_BB_UNIQUE(std::string& identifier)
{
	// BB_UNIQUE REGEX
	const std::regex pattern("^BBG[0-9A-Z]{9}$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}

	return false;
}

bool Request::Validate_SEDOL(std::string& identifier)
{
	// SEDOL REGEX
	const std::regex pattern("^[0-9A-Z]{7}$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}

	return false;
}

bool Request::Validate_COMMON(std::string& identifier)
{
	// COMMON REGEX
	const std::regex pattern("^[A-Z0-9]+$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}

	return false;
}

bool Request::Validate_WERTPAPIER(std::string& identifier)
{
	// WERTPAPIER REGEX
	const std::regex pattern("^[0-9A-Z]{6}$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}

	return false;
}

bool Request::Validate_COMPOSITE_ID_BB_GLOBAL(std::string& identifier)
{
	// COMPOSITE_ID_BB_GLOBAL REGEX
	const std::regex pattern("^BBG[0-9A-Z]{9}$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}

	return false;
}

bool Request::Validate_TICKER(std::string& identifier)
{
	const std::regex pattern("([A-Za-z]{1,5})(-[A-Za-z]{1,2})?");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}
	return false;
}

bool Request::Validate_CUSIP(std::string& identifier)
{
	// CUSIP REGEX
	const std::regex pattern("^[0-9A-Z]{9}$");
	if (identifier.empty())
		return false;

	if (std::regex_match(identifier, pattern))
	{
		return true;
	}

	return false;
}

void Request::GetVec()
{
	m_Identifiers = m_fileState.GetVec();
}

nlohmann::json Request::GetResponse()
{
	if (!m_sResponse.empty())
	{
		ParseResponse();
		return m_sResponse;
	}
	return nullptr;
}

void Request::ClearResponse()
{
	m_IdentifierPairs.clear();
	m_sResponse.clear();
	m_RequestBody.clear();
	m_AllRequestBody.clear();
}

void Request::ParseResponse()
{

	size_t counter = 0;

	auto& inner_json = m_sResponse;

	for (auto& elem : inner_json)
	{
		elem["exchCode"] = (m_AllRequestBody[counter]["exchCode"]);
		elem["idValue"] = (m_AllRequestBody[counter]["idValue"]);
		elem["idType"] = (m_AllRequestBody[counter]["idType"]);
		counter++;
	}

}

std::vector<std::pair<std::string, Request::IdentifierType>> Request::GetIdentifierType()
{
	for (auto& elem : m_Identifiers)
	{
		IdentifierType type = IdentifierType::NONE;
		std::string sType = "";

		if (Validate_ISIN(elem))
		{
			type = IdentifierType::ID_ISIN;
			sType = "ID_ISIN";
		}
		else if (Validate_TICKER(elem))
		{
			type = IdentifierType::TICKER;
			sType = "TICKER";
		}
		else if (Validate_CUSIP(elem))
		{
			type = IdentifierType::ID_CUSIP;
			sType = "CUSIP";
		}
		else if (Validate_COMPOSITE_ID_BB_GLOBAL(elem))
		{
			type = IdentifierType::COMPOSITE_ID_BB_GLOBAL;
			sType = "FIGI";
		}
		else if (Validate_SEDOL(elem))
		{
			type = IdentifierType::ID_SEDOL;
			sType = "SEDOL";
		}

		m_IdentifierPairs.push_back(make_pair(elem, type));
		//std::cout << elem << " [" << sType << "]" << std::endl;

	}
	return m_IdentifierPairs;
}


