#include "Request.h"

using namespace nlohmann;

Request::Request(FileState& fileState)
	: m_fileState(fileState)
	, r{}
	, m_IdentifierPairs{}
	, m_IdentifierType{ IdentifierType::NONE }
	, m_sResponse{json::array()}
	, m_RequestBody{json::array()}
	, m_AllRequestBody{}
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
	json jsonBody_request;
	json requestBody = json::array();

	//json jsonArray;
	json badResponse = json::array({"warning", "No identifier found."});

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


		if (!id_context_type.empty())
		{
			if (idType == "TICKER")
			{
				process_ticker(idValue);
				jsonBody =
				{
					{"idType", idType},
					{"idValue", idValue},
					{id_context_type, id_context_value},
					{"includeUnlistedEquities", true}
				};
				//requestBody.push_back(jsonBody); // Pushes back jsonBody to requestBody json array used in the actual post request.
			}
			else if (idType != "NONE")
			{
				jsonBody =
				{
					{"idType", idType},
					{"idValue", idValue},
					{id_context_type, id_context_value},
					{"includeUnlistedEquities", true}
				};
				//requestBody.push_back(jsonBody); // Pushes back jsonBody to requestBody json array used in the actual post request.
			}

			jsonBody_request =
			{
				{"idType", idType},
				{"idValue", idValue},
				{id_context_type, id_context_value},
				{"includeUnlistedEquities", true}
			};
			requestBody.push_back(jsonBody); // Pushes back jsonBody to requestBody json array used in the actual post request.
			m_AllRequestBody.push_back(jsonBody_request);
		}
		else
		{
			if (idType == "TICKER")
			{
				process_ticker(idValue);
				jsonBody =
				{
					{"idType", idType},
					{"idValue", idValue},
					{"includeUnlistedEquities", true}
				};


				//requestBody.push_back(jsonBody); // Pushes back jsonBody to requestBody json array used in the actual post request.
			}
			else if (idType != "NONE")
			{
				jsonBody =
				{
					{"idType", idType},
					{"idValue", idValue},
					{"includeUnlistedEquities", true}
				};

				//requestBody.push_back(jsonBody); // Pushes back jsonBody to requestBody json array used in the actual post request.
			}

			jsonBody_request =
			{
				{"idType", idType},
				{"idValue", idValue},
				{"includeUnlistedEquities", true}
			};
			requestBody.push_back(jsonBody); // Pushes back jsonBody to requestBody json array used in the actual post request.
			m_AllRequestBody.push_back(jsonBody_request);
		}



		if (job_count >= 26 && timer.ElapsedSec() <= 6)
		{
			api_cooldown = true;
			while (api_cooldown)
			{
				if (timer.ElapsedSec() > 6)
				{
					job_count = 0;
					timer.Stop();
					timer.Start();
					api_cooldown = false;
				}
			}
		}


		if (requestBody.size() == 100 || (m_IdentifierPairs.size() - counter) == 0)
		{
			r = cpr::Post(
				cpr::Url{ "https://api.openfigi.com/v3/mapping" },
				cpr::Header{
					{"Content-Type", "application/json"},
					{"X-OPENFIGI-APIKEY", m_apikey}
				},
				cpr::Body{ requestBody.dump() }
			);
			if (r.status_code != 200)
			{
				std::cout << "Error " << r.status_code << " | " << r.text << std::endl;
				return;
			}
			job_count++;

			json responseJson = json::parse(r.text);
			m_sResponse.insert(m_sResponse.end(), responseJson.begin(), responseJson.end());
			m_RequestBody.insert(m_RequestBody.end(), responseJson.begin(), responseJson.end());
			requestBody.clear();

		}
	}
}


std::string Request::validate_base_identifier(const std::string& identifier)
{
	std::string idType = "";

	const std::regex ticker_pattern("([A-Za-z]{1,5})([-./]?[A-Za-z]{1,2})?");
	const std::regex isin_pattern("^(?!BBG)[A-Z]{2}[A-Z0-9]{9}[0-9]$");
	const std::regex sedol_pattern("^[0-9B-DF-HJ-NP-TV-Z]{6}[0-9]$");
	const std::regex cusip_pattern("^[0-9]{3}[0-9a-zA-Z]{5}[0-9]");
	const std::regex wkn_pattern("^[A-HJ-NP-Z0-9]{6}$");
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
			idType = "ID_BB_GLOBAL";
		}
		else if (std::regex_match(identifier, wkn_pattern))
		{
			idType = "ID_WERTPAPIER";
		}
		else
		{
			idType = "NONE";
		}
		return idType;
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
	size_t dotPos = str.find('.');
	if (dotPos != std::string::npos) {
		size_t afterDot = dotPos + 1;
		size_t charsAfterDot = str.size() - afterDot;

		if (charsAfterDot == 0) {
			// Case 3: '.' at the end, remove it
			str.erase(dotPos, 1);
		}
		else if (charsAfterDot == 1) {
			// Case 1: One character after '.', replace '.' with '/'
			str[dotPos] = '/';
		}
		else if (charsAfterDot > 1) {
			// Case 2: Multiple characters after '.', replace '.' with '-', keep only immediate character
			str.replace(dotPos, charsAfterDot + 1, "-" + str.substr(afterDot, 1));
		}
	}
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
	//m_Identifiers.clear();
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
		////elem["exchCode"] = (m_AllRequestBody[counter]["exchCode"]);
		//elem["idValue"] = (m_AllRequestBody[counter]["idValue"]);
		//elem["idType"] = (m_AllRequestBody[counter]["idType"]);
		//counter++;
				// Check if the current counter is within bounds for m_AllRequestBody
		if (counter >= m_AllRequestBody.size())
		{
			// Log an error, throw an exception, or handle this case as necessary
			//std::cout << "Warning: m_AllRequestBody index out of range at ParseResponse()." << std::endl;
			break; // Stop the loop to avoid out-of-range access
		}
		for (auto it = m_AllRequestBody[counter].begin(); it != m_AllRequestBody[counter].end(); ++it)
		{
			// Assign the value from m_AllRequestBody to the corresponding key in elem
			if (it.key() != "includeUnlistedEquities")
			{
				elem[it.key()] = it.value();
			}
			//elem[it.key()] = it.value();
		}
		counter++;
	}

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


	// Looping through each element in the input_vector, which is each row.
	for (auto elem : m_Identifiers)
	{
		std::istringstream iss(elem);

		std::string base_identifier = "";
		std::string contextual_identifier = "";

		iss >> base_identifier >> contextual_identifier;
		m_IdentifierPairs.push_back(make_pair(base_identifier, contextual_identifier));
	}

	return m_IdentifierPairs;
}