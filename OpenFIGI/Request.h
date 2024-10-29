#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cpr/cpr.h>
#include <json.hpp>
#include <algorithm>
#include <cctype>
#include "states/FileState.h"
#include "utilities/Timer.h"

class FileState;

class Request
{
public:
	//Request(const std::vector<std::string>& identifiers);
	Request(FileState& fileState);
	~Request() = default;

	//void GetIdentifiers(std::vector<std::string>& identifiers);
	bool Validate_ISIN(std::string& identifier);
	bool Validate_BB_UNIQUE(std::string& identifier);
	bool Validate_SEDOL(std::string& identifier);
	bool Validate_COMMON(std::string& identifier);
	bool Validate_WERTPAPIER(std::string& identifier);
	bool Validate_COMPOSITE_ID_BB_GLOBAL(std::string& identifier);
	bool Validate_TICKER(std::string& identifier);
	bool Validate_CUSIP(std::string& identifier);
	
	bool isAllLetters(const std::string& str) {
		return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isalpha(c); });
	}

	bool isAllDigits(const std::string& str) {
		return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
	}

	enum class IdentifierType {
		ID_ISIN = 0, ID_BB_UNIQUE, ID_SEDOL, ID_COMMON, ID_WERTPAPIER, ID_CUSIP, ID_CINS,
		ID_BB, ID_BB_8_CHR, ID_TRACE, ID_ITALY, ID_EXCH_SYMBOL, ID_FULL_EXCHANGE_SYMBOL,
		COMPOSITE_ID_BB_GLOBAL, ID_BB_GLOBAL_SHARE_CLASS_LEVEL, ID_BB_GLOBAL,
		ID_BB_SEC_NUM_DES, TICKER, BASE_TICKER, ID_CUSIP_8_CHR, OCC_SYMBOL,
		UNIQUE_ID_FUT_OPT, OPRA_SYMBOL, TRADING_SYSTEM_IDENTIFIER, ID_SHORT_CODE,
		VENDOR_INDEX_CODE, NONE
	};

	void GetVec();
	std::vector<std::pair<std::string, IdentifierType>> GetIdentifierType();	
	void GetIdentifiers();

	nlohmann::json GetResponse();

	void ParseResponse();


	

private:
	const std::string API_KEY = "853a5b1c-9ee7-45a9-85fe-67504db399b0";

	FileState& m_fileState;
	std::vector<std::string> m_Identifiers;
	std::vector<std::pair<std::string, Request::IdentifierType>> m_IdentifierPairs;


	IdentifierType m_IdentifierType;
	nlohmann::json m_sResponse;
	nlohmann::json m_RequestBody;
	nlohmann::json m_AllRequestBody;
	//HWND hwnd;



	/*
	Send your API key using the HTTP header X-OPENFIGI-APIKEY. With a valid API key, you will be subject to a higher rate-limit and request size.

	curl 'https://api.openfigi.com/v3/mapping' \
	--request POST \
	--header 'Content-Type: application/json' \
	--header 'X-OPENFIGI-APIKEY: abcdefghijklmnopqrstuvwxyz' \
	--data '[{"idType":"ID_WERTPAPIER","idValue":"851399"}]'

	cpr::Response r = cpr::Post(cpr::Url{"http://www.httpbin.org/post"},
				   cpr::Body{"This is raw POST data"},
				   cpr::Header{{"Content-Type", "text/plain"}});
					std::cout << r.text << std::endl;

	*/

};