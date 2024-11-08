#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cpr/cpr.h>
#include <json.hpp>
#include <regex>
#include <algorithm>
#include <cctype>
#include <cassert>
#include "states/FileState.h"
#include "utilities/Timer.h"

class FileState;

class Request
{
public:
	Request(FileState& fileState);
	~Request() = default;

	std::string validate_base_identifier(const std::string& identifier);
	std::string validate_context_identifier(const std::string& context_identifier);
	
	bool isAllLetters(const std::string& str) {
		return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isalpha(c); });
	}

	bool isAllDigits(const std::string& str) {
		return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
	}

	void process_ticker(std::string& str);

	enum class IdentifierType {
		NONE = 0, ID_ISIN, ID_BB_UNIQUE, ID_SEDOL, ID_COMMON, ID_WERTPAPIER, ID_CUSIP, ID_CINS,
		ID_BB, ID_BB_8_CHR, ID_TRACE, ID_ITALY, ID_EXCH_SYMBOL, ID_FULL_EXCHANGE_SYMBOL,
		COMPOSITE_ID_BB_GLOBAL, ID_BB_GLOBAL_SHARE_CLASS_LEVEL, ID_BB_GLOBAL,
		ID_BB_SEC_NUM_DES, TICKER, BASE_TICKER, ID_CUSIP_8_CHR, OCC_SYMBOL,
		UNIQUE_ID_FUT_OPT, OPRA_SYMBOL, TRADING_SYSTEM_IDENTIFIER, ID_SHORT_CODE,
		VENDOR_INDEX_CODE
	};


	//std::vector<std::pair<std::string, IdentifierType>> GetIdentifierType();
	std::vector<std::pair<std::string, std::string>> GetIdentifierType();
	void GetVec();	
	void GetIdentifiers();

	nlohmann::json GetResponse();
	void ClearResponse();
	void ParseResponse();


	std::string const get_apikey();
	void set_apikey(std::wstring api_key);
	

private:

	cpr::Response r;

	std::string m_apikey;

	FileState& m_fileState;
	std::vector<std::string> m_Identifiers;
	std::vector<std::pair<std::string, std::string>> m_IdentifierPairs;


	IdentifierType m_IdentifierType;

	nlohmann::json m_current_request_valid;
	nlohmann::json m_all_requests_valid;

	nlohmann::json m_response;
	nlohmann::json m_bad_response;

	nlohmann::json m_combined;




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