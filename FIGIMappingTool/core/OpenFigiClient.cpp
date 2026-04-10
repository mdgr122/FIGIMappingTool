#include "OpenFigiClient.h"
#include <algorithm>
#include <cpr/cpr.h>
#include <regex>
#include <thread>

namespace figi
{

// ─── IdType string conversion ───────────────────────────────────────────────
constexpr std::string_view to_string(IdType t) noexcept
{
    switch (t)
    {
        case IdType::ID_ISIN:
            return "ID_ISIN";
        case IdType::ID_BB_UNIQUE:
            return "ID_BB_UNIQUE";
        case IdType::ID_SEDOL:
            return "ID_SEDOL";
        case IdType::ID_COMMON:
            return "ID_COMMON";
        case IdType::ID_WERTPAPIER:
            return "ID_WERTPAPIER";
        case IdType::ID_CUSIP:
            return "ID_CUSIP";
        case IdType::ID_CINS:
            return "ID_CINS";
        case IdType::ID_BB:
            return "ID_BB";
        case IdType::ID_BB_8_CHR:
            return "ID_BB_8_CHR";
        case IdType::ID_TRACE:
            return "ID_TRACE";
        case IdType::ID_ITALY:
            return "ID_ITALY";
        case IdType::ID_EXCH_SYMBOL:
            return "ID_EXCH_SYMBOL";
        case IdType::ID_FULL_EXCHANGE_SYMBOL:
            return "ID_FULL_EXCHANGE_SYMBOL";
        case IdType::COMPOSITE_ID_BB_GLOBAL:
            return "COMPOSITE_ID_BB_GLOBAL";
        case IdType::ID_BB_GLOBAL_SHARE_CLASS_LEVEL:
            return "ID_BB_GLOBAL_SHARE_CLASS_LEVEL";
        case IdType::ID_BB_GLOBAL:
            return "ID_BB_GLOBAL";
        case IdType::ID_BB_SEC_NUM_DES:
            return "ID_BB_SEC_NUM_DES";
        case IdType::TICKER:
            return "TICKER";
        case IdType::BASE_TICKER:
            return "BASE_TICKER";
        case IdType::ID_CUSIP_8_CHR:
            return "ID_CUSIP_8_CHR";
        case IdType::OCC_SYMBOL:
            return "OCC_SYMBOL";
        case IdType::UNIQUE_ID_FUT_OPT:
            return "UNIQUE_ID_FUT_OPT";
        case IdType::OPRA_SYMBOL:
            return "OPRA_SYMBOL";
        case IdType::TRADING_SYSTEM_IDENTIFIER:
            return "TRADING_SYSTEM_IDENTIFIER";
        case IdType::ID_SHORT_CODE:
            return "ID_SHORT_CODE";
        case IdType::VENDOR_INDEX_CODE:
            return "VENDOR_INDEX_CODE";
    }
    return "UNKNOWN";
}

std::optional<IdType> parse_id_type(std::string_view s) noexcept
{
    static const auto map = []
    {
        std::unordered_map<std::string_view, IdType> m;
        auto reg = [&](IdType t) { m[to_string(t)] = t; };
        reg(IdType::ID_ISIN);
        reg(IdType::ID_BB_UNIQUE);
        reg(IdType::ID_SEDOL);
        reg(IdType::ID_COMMON);
        reg(IdType::ID_WERTPAPIER);
        reg(IdType::ID_CUSIP);
        reg(IdType::ID_CINS);
        reg(IdType::ID_BB);
        reg(IdType::ID_BB_8_CHR);
        reg(IdType::ID_TRACE);
        reg(IdType::ID_ITALY);
        reg(IdType::ID_EXCH_SYMBOL);
        reg(IdType::ID_FULL_EXCHANGE_SYMBOL);
        reg(IdType::COMPOSITE_ID_BB_GLOBAL);
        reg(IdType::ID_BB_GLOBAL_SHARE_CLASS_LEVEL);
        reg(IdType::ID_BB_GLOBAL);
        reg(IdType::ID_BB_SEC_NUM_DES);
        reg(IdType::TICKER);
        reg(IdType::BASE_TICKER);
        reg(IdType::ID_CUSIP_8_CHR);
        reg(IdType::OCC_SYMBOL);
        reg(IdType::UNIQUE_ID_FUT_OPT);
        reg(IdType::OPRA_SYMBOL);
        reg(IdType::TRADING_SYSTEM_IDENTIFIER);
        reg(IdType::ID_SHORT_CODE);
        reg(IdType::VENDOR_INDEX_CODE);
        return m;
    }();
    if (auto it = map.find(s); it != map.end())
        return it->second;
    return std::nullopt;
}

// ─── Instrument ─────────────────────────────────────────────────────────────

Instrument Instrument::from_json(const nlohmann::json& j)
{
    auto get = [&](const char* key) -> std::string
    {
        if (j.contains(key) && !j[key].is_null())
            return j[key].get<std::string>();
        return { };
    };
    return {.figi = get("figi"), .name = get("name"), .ticker = get("ticker"), .exchCode = get("exchCode"),
            .securityType = get("securityType"), .marketSector = get("marketSector"),
            .securityType2 = get("securityType2"), .securityDescription = get("securityDescription"),
            .compositeFIGI = get("compositeFIGI"), .shareClassFIGI = get("shareClassFIGI"),};
}

// ─── MappingJob ─────────────────────────────────────────────────────────────

nlohmann::json MappingJob::to_json() const
{
    nlohmann::json j;
    j["idType"] = std::string(to_string(idType));
    j["idValue"] = idValue;

    auto set = [&](const char* key, const auto& opt)
    {
        if (opt.has_value())
            j[key] = *opt;
    };
    auto set_iv = [&](const char* key, const auto& opt)
    {
        if (opt.has_value())
            j[key] = opt->to_json();
    };

    set("exchCode", exchCode);
    set("micCode", micCode);
    set("currency", currency);
    set("marketSecDes", marketSecDes);
    set("securityType", securityType);
    set("securityType2", securityType2);
    set("includeUnlistedEquities", includeUnlistedEquities);
    set("optionType", optionType);
    set("stateCode", stateCode);
    set_iv("strike", strike);
    set_iv("contractSize", contractSize);
    set_iv("coupon", coupon);
    set_iv("expiration", expiration);
    set_iv("maturity", maturity);

    return j;
}

Result<void> MappingJob::validate() const
{
    if (exchCode && micCode)
    {
        return std::unexpected(Error{ErrorCode::InvalidInput, "exchCode and micCode are mutually exclusive", { }});
    }
    if (idValue.empty())
    {
        return std::unexpected(Error{ErrorCode::InvalidInput, "idValue must not be empty", { }});
    }
    return { };
}

// ─── SearchRequest ──────────────────────────────────────────────────────────

nlohmann::json SearchRequest::to_json() const
{
    nlohmann::json j;
    auto set = [&](const char* key, const auto& opt)
    {
        if (opt.has_value())
            j[key] = *opt;
    };
    auto set_iv = [&](const char* key, const auto& opt)
    {
        if (opt.has_value())
            j[key] = opt->to_json();
    };

    set("query", query);
    set("start", start);
    set("exchCode", exchCode);
    set("micCode", micCode);
    set("currency", currency);
    set("marketSecDes", marketSecDes);
    set("securityType", securityType);
    set("securityType2", securityType2);
    set("includeUnlistedEquities", includeUnlistedEquities);
    set("optionType", optionType);
    set("stateCode", stateCode);
    set_iv("strike", strike);
    set_iv("contractSize", contractSize);
    set_iv("coupon", coupon);
    set_iv("expiration", expiration);
    set_iv("maturity", maturity);

    return j;
}

// ─── RateLimiter ────────────────────────────────────────────────────────────

RateLimiter::RateLimiter(int max_requests, std::chrono::seconds window)
    : m_max(max_requests)
    , m_window(window)
{
    m_timestamps.reserve(static_cast<size_t>(max_requests));
}

void RateLimiter::reconfigure(int max_requests, std::chrono::seconds window)
{
    std::lock_guard lock(m_mutex);
    m_max = max_requests;
    m_window = window;
    m_timestamps.clear();
    m_server_reset.reset();
    m_server_remaining.reset();
}

void RateLimiter::acquire()
{
    using clock = std::chrono::steady_clock;
    std::unique_lock lock(m_mutex);

    while (true)
    {
        auto now = clock::now();

        // Respect server-reported rate limit state.
        if (m_server_remaining && *m_server_remaining <= 0 && m_server_reset)
        {
            auto wait = *m_server_reset - now;
            if (wait > std::chrono::seconds::zero())
            {
                lock.unlock();
                std::this_thread::sleep_for(wait + std::chrono::milliseconds(100));
                lock.lock();
                m_server_remaining.reset();
                m_server_reset.reset();
                continue;
            }
        }

        // Purge timestamps outside the window.
        auto cutoff = now - m_window;
        std::erase_if(m_timestamps, [cutoff](auto& tp) { return tp < cutoff; });

        if (static_cast<int>(m_timestamps.size()) < m_max)
        {
            m_timestamps.push_back(now);
            return;
        }

        // Wait until the oldest timestamp exits the window.
        auto wait = m_timestamps.front() + m_window - now + std::chrono::milliseconds(50);
        lock.unlock();
        std::this_thread::sleep_for(wait);
        lock.lock();
    }
}

void RateLimiter::update_from_headers(int remaining, std::chrono::seconds reset)
{
    std::lock_guard lock(m_mutex);
    m_server_remaining = remaining;
    m_server_reset = std::chrono::steady_clock::now() + reset;
}

// ─── OpenFigiClient ─────────────────────────────────────────────────────────

OpenFigiClient::OpenFigiClient(ClientConfig config)
    : m_config(std::move(config))
    , m_mapping_limiter(m_config.api_key.empty() ? 25 : 25,
                        m_config.api_key.empty() ? std::chrono::seconds(60) : std::chrono::seconds(6))
    , m_search_limiter(m_config.api_key.empty() ? 5 : 20, std::chrono::seconds(60))
{}

void OpenFigiClient::set_api_key(std::string_view key)
{
    std::lock_guard lock(m_config_mutex);
    m_config.api_key = std::string(key);

    // Reconfigure rate limiters for the new auth state.
    bool has_key = !m_config.api_key.empty();
    m_mapping_limiter.reconfigure(25, has_key ? std::chrono::seconds(6) : std::chrono::seconds(60));
    m_search_limiter.reconfigure(has_key ? 20 : 5, std::chrono::seconds(60));
}

std::string OpenFigiClient::get_api_key() const
{
    std::lock_guard lock(m_config_mutex);
    return m_config.api_key;
}

std::string OpenFigiClient::current_api_key() const
{
    std::lock_guard lock(m_config_mutex);
    return m_config.api_key;
}

size_t OpenFigiClient::batch_size() const { return current_api_key().empty() ? 10 : 100; }

void OpenFigiClient::update_limiter(RateLimiter& limiter, const HttpResponse& resp)
{
    if (resp.ratelimit_remaining >= 0 && resp.ratelimit_reset >= 0)
    {
        limiter.update_from_headers(resp.ratelimit_remaining, std::chrono::seconds(resp.ratelimit_reset));
    }
}

Result<OpenFigiClient::HttpResponse>
OpenFigiClient::post(std::string_view endpoint, const nlohmann::json& body, RateLimiter& limiter)
{
    auto api_key = current_api_key();
    return request_with_retry([&, api_key]() -> Result<HttpResponse>
                              {
                                  auto url = std::format("{}{}", BASE_URL, endpoint);

                                  cpr::Header headers{{"Content-Type", "application/json"}};
                                  if (!api_key.empty())
                                  {
                                      headers["X-OPENFIGI-APIKEY"] = api_key;
                                  }

                                  auto r = cpr::Post(cpr::Url{url}, cpr::Header{headers}, cpr::Body{body.dump()});

                                  if (r.error)
                                  {
                                      return std::unexpected(Error{ErrorCode::NetworkError,
                                                                   std::format("Network error: {}", r.error.message),
                                                                   { }});
                                  }

                                  HttpResponse resp{.status = static_cast<int>(r.status_code),
                                                    .body = std::move(r.text),};

                                  if (auto it = r.header.find("ratelimit-remaining"); it != r.header.end())
                                  {
                                      try
                                      {
                                          resp.ratelimit_remaining = std::stoi(it->second);
                                      }
                                      catch (...)
                                      {}
                                  }
                                  if (auto it = r.header.find("ratelimit-reset"); it != r.header.end())
                                  {
                                      try
                                      {
                                          resp.ratelimit_reset = std::stoi(it->second);
                                      }
                                      catch (...)
                                      {}
                                  }

                                  return resp;
                              },
                              limiter);
}

Result<OpenFigiClient::HttpResponse> OpenFigiClient::get(std::string_view endpoint, RateLimiter& limiter)
{
    auto api_key = current_api_key();
    return request_with_retry([&, api_key]() -> Result<HttpResponse>
                              {
                                  auto url = std::format("{}{}", BASE_URL, endpoint);

                                  cpr::Header headers;
                                  if (!api_key.empty())
                                  {
                                      headers["X-OPENFIGI-APIKEY"] = api_key;
                                  }

                                  auto r = cpr::Get(cpr::Url{url}, cpr::Header{headers});

                                  if (r.error)
                                  {
                                      return std::unexpected(Error{ErrorCode::NetworkError,
                                                                   std::format("Network error: {}", r.error.message),
                                                                   { }});
                                  }

                                  HttpResponse resp{.status = static_cast<int>(r.status_code),
                                                    .body = std::move(r.text),};

                                  if (auto it = r.header.find("ratelimit-remaining"); it != r.header.end())
                                  {
                                      try
                                      {
                                          resp.ratelimit_remaining = std::stoi(it->second);
                                      }
                                      catch (...)
                                      {}
                                  }
                                  if (auto it = r.header.find("ratelimit-reset"); it != r.header.end())
                                  {
                                      try
                                      {
                                          resp.ratelimit_reset = std::stoi(it->second);
                                      }
                                      catch (...)
                                      {}
                                  }

                                  return resp;
                              },
                              limiter);
}

Result<OpenFigiClient::HttpResponse>
OpenFigiClient::request_with_retry(std::function<Result<HttpResponse>()> do_request, RateLimiter& limiter)
{
    int max_retries = [&]
    {
        std::lock_guard lock(m_config_mutex);
        return m_config.max_retries;
    }();

    for (int attempt = 0; attempt <= max_retries; ++attempt)
    {
        limiter.acquire();

        auto result = do_request();
        if (!result)
            return result;

        auto& resp = *result;
        update_limiter(limiter, resp);

        if (resp.status == 200)
            return result;

        auto err = Error{static_cast<ErrorCode>(resp.status), std::format("HTTP {}", resp.status), resp.body};

        if (!err.is_retryable() || attempt == max_retries)
        {
            return std::unexpected(std::move(err));
        }

        // Exponential backoff, respect rate limit-reset if available.
        auto wait = std::chrono::seconds(1 << attempt);
        if (resp.ratelimit_reset > 0)
        {
            wait = std::max(wait, std::chrono::seconds(resp.ratelimit_reset));
        }
        std::this_thread::sleep_for(wait);
    }

    return std::unexpected(Error{ErrorCode::ServerError, "Max retries exceeded", { }});
}

Result<nlohmann::json> OpenFigiClient::parse_body(const HttpResponse& resp)
{
    try
    {
        return nlohmann::json::parse(resp.body);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return std::unexpected(Error{ErrorCode::ParseError, std::format("JSON parse error: {}", e.what()), resp.body});
    }
}

// ─── Mapping ────────────────────────────────────────────────────────────────

Result<std::vector<MappingResult>> OpenFigiClient::map(std::vector<MappingJob> jobs)
{
    for (size_t i = 0; i < jobs.size(); ++i)
    {
        if (auto v = jobs[i].validate(); !v)
        {
            return std::unexpected(Error{ErrorCode::InvalidInput, std::format("Job {}: {}", i, v.error().message),
                                         { }});
        }
    }

    const size_t bs = batch_size();
    std::vector<MappingResult> all_results;
    all_results.reserve(jobs.size());

    auto on_progress = [&]
    {
        std::lock_guard lock(m_config_mutex);
        return m_config.on_progress;
    }();

    for (size_t offset = 0; offset < jobs.size(); offset += bs)
    {
        auto end = std::min(offset + bs, jobs.size());
        nlohmann::json batch = nlohmann::json::array();
        for (size_t i = offset; i < end; ++i)
        {
            batch.push_back(jobs[i].to_json());
        }

        auto resp = post("/v3/mapping", batch, m_mapping_limiter);
        if (!resp)
            return std::unexpected(resp.error());

        auto parsed = parse_body(*resp);
        if (!parsed)
            return std::unexpected(parsed.error());

        auto& arr = *parsed;
        if (!arr.is_array() || arr.size() != (end - offset))
        {
            return std::unexpected(Error{ErrorCode::ParseError,
                                         std::format("Expected {} results, got {}",
                                                     end - offset,
                                                     arr.is_array() ? arr.size() : 0),
                                         arr.dump()});
        }

        for (size_t i = 0; i < arr.size(); ++i)
        {
            MappingResult mr;
            mr.original_request = jobs[offset + i].to_json();

            if (arr[i].contains("data") && arr[i]["data"].is_array())
            {
                std::vector<Instrument> instruments;
                instruments.reserve(arr[i]["data"].size());
                for (const auto& item : arr[i]["data"])
                {
                    instruments.push_back(Instrument::from_json(item));
                }
                mr.outcome = std::move(instruments);
            }
            else if (arr[i].contains("warning"))
            {
                mr.outcome.emplace<1>(arr[i]["warning"].get<std::string>());
            }
            else if (arr[i].contains("error"))
            {
                mr.outcome.emplace<2>(arr[i]["error"].get<std::string>());
            }
            else
            {
                mr.outcome.emplace<2>("Unknown response structure");
            }

            all_results.push_back(std::move(mr));
        }

        if (on_progress)
        {
            on_progress(std::min(end, jobs.size()), jobs.size());
        }
    }

    return all_results;
}

Result<std::vector<MappingResult>>
OpenFigiClient::map(std::string_view id_value, std::optional<std::string_view> context)
{
    auto detected = detect_id_type(id_value);
    if (!detected)
    {
        return std::unexpected(Error{ErrorCode::InvalidInput, std::format("Cannot detect idType for '{}'", id_value),
                                     { }});
    }

    MappingJob job{.idType = *detected, .idValue = std::string(id_value)};
    job.includeUnlistedEquities = true;

    if (context)
    {
        auto ctx = *context;
        if (ctx.size() == 2)
            job.exchCode = std::string(ctx);
        else if (ctx.size() == 3)
            job.currency = std::string(ctx);
        else if (ctx.size() == 4)
            job.micCode = std::string(ctx);
    }

    return map(std::vector{std::move(job)});
}

// ─── Search / Filter ────────────────────────────────────────────────────────

static Result<SearchResponse> parse_search_response(Result<nlohmann::json> parsed)
{
    if (!parsed)
        return std::unexpected(parsed.error());

    auto& j = *parsed;
    SearchResponse sr;

    if (j.contains("error"))
    {
        return std::unexpected(Error{ErrorCode::BadRequest, j["error"].get<std::string>(), j.dump()});
    }

    if (j.contains("data") && j["data"].is_array())
    {
        sr.data.reserve(j["data"].size());
        for (const auto& item : j["data"])
        {
            sr.data.push_back(Instrument::from_json(item));
        }
    }

    if (j.contains("next"))
        sr.next = j["next"].get<std::string>();
    if (j.contains("total"))
        sr.total = j["total"].get<int64_t>();

    return sr;
}

Result<SearchResponse> OpenFigiClient::search(SearchRequest req)
{
    auto resp = post("/v3/search", req.to_json(), m_search_limiter);
    if (!resp)
        return std::unexpected(resp.error());
    return parse_search_response(parse_body(*resp));
}

Result<std::vector<Instrument>> OpenFigiClient::search_all(SearchRequest req, int max_pages)
{
    std::vector<Instrument> all;
    for (int page = 0; page < max_pages; ++page)
    {
        auto result = search(req);
        if (!result)
            return std::unexpected(result.error());

        auto& sr = *result;
        all.insert(all.end(), std::make_move_iterator(sr.data.begin()), std::make_move_iterator(sr.data.end()));

        if (!sr.next)
            break;
        req.start = *sr.next;
    }
    return all;
}

Result<SearchResponse> OpenFigiClient::filter(SearchRequest req)
{
    auto resp = post("/v3/filter", req.to_json(), m_search_limiter);
    if (!resp)
        return std::unexpected(resp.error());
    return parse_search_response(parse_body(*resp));
}

Result<std::vector<Instrument>> OpenFigiClient::filter_all(SearchRequest req, int max_pages)
{
    std::vector<Instrument> all;
    for (int page = 0; page < max_pages; ++page)
    {
        auto result = filter(req);
        if (!result)
            return std::unexpected(result.error());

        auto& sr = *result;
        all.insert(all.end(), std::make_move_iterator(sr.data.begin()), std::make_move_iterator(sr.data.end()));

        if (!sr.next)
            break;
        req.start = *sr.next;
    }
    return all;
}

// ─── Enum values ────────────────────────────────────────────────────────────

Result<std::vector<std::string>> OpenFigiClient::get_enum_values(std::string_view key)
{
    auto endpoint = std::format("/v3/mapping/values/{}", key);
    auto resp = get(endpoint, m_mapping_limiter);
    if (!resp)
        return std::unexpected(resp.error());

    auto parsed = parse_body(*resp);
    if (!parsed)
        return std::unexpected(parsed.error());

    auto& j = *parsed;
    if (j.contains("values") && j["values"].is_array())
    {
        std::vector<std::string> vals;
        vals.reserve(j["values"].size());
        for (const auto& v : j["values"])
            vals.push_back(v.get<std::string>());
        return vals;
    }

    return std::unexpected(Error{ErrorCode::ParseError, "No 'values' array in response", j.dump()});
}

// ─── Identifier detection ───────────────────────────────────────────────────

std::optional<IdType> OpenFigiClient::detect_id_type(std::string_view value)
{
    if (value.empty())
        return std::nullopt;
    std::string s(value);

    static const std::regex figi_re(R"(^BBG[0-9A-Z]{9}$)");
    if (std::regex_match(s, figi_re))
        return IdType::ID_BB_GLOBAL;

    static const std::regex isin_re(R"(^(?!BBG)[A-Z]{2}[A-Z0-9]{9}[0-9]$)");
    if (std::regex_match(s, isin_re))
        return IdType::ID_ISIN;

    static const std::regex sedol_re(R"(^[0-9B-DF-HJ-NP-TV-Z]{6}[0-9]$)");
    if (std::regex_match(s, sedol_re))
        return IdType::ID_SEDOL;

    static const std::regex cusip_re(R"(^[0-9]{3}[0-9A-Z]{6}$)");
    if (std::regex_match(s, cusip_re))
        return IdType::ID_CUSIP;

    static const std::regex ticker_re(R"(^(?!BBG[0-9A-Z]{9}$)([A-Za-z0-9]+)([-./]?[A-Za-z]{1,2})?( Index)?$)");
    if (std::regex_match(s, ticker_re))
        return IdType::TICKER;

    return std::nullopt;
}

// ─── Convenience lookups ────────────────────────────────────────────────────

static Result<std::vector<Instrument>> extract_first_data(Result<std::vector<MappingResult>> results)
{
    if (!results)
        return std::unexpected(results.error());
    if (results->empty() || !results->front().has_data())
        return std::vector<Instrument>{ };
    return results->front().data();
}

Result<std::vector<Instrument>> OpenFigiClient::lookup_figi(std::string_view figi)
{
    return extract_first_data(
        map(std::vector{MappingJob{.idType = IdType::ID_BB_GLOBAL, .idValue = std::string(figi)}}));
}

Result<std::vector<Instrument>>
OpenFigiClient::lookup_ticker(std::string_view ticker, std::optional<std::string_view> exch_code)
{
    MappingJob job{.idType = IdType::TICKER, .idValue = std::string(ticker)};
    if (exch_code)
        job.exchCode = std::string(*exch_code);
    return extract_first_data(map(std::vector{std::move(job)}));
}

Result<std::vector<Instrument>> OpenFigiClient::lookup_isin(std::string_view isin)
{
    return extract_first_data(map(std::vector{MappingJob{.idType = IdType::ID_ISIN, .idValue = std::string(isin)}}));
}

Result<std::vector<Instrument>> OpenFigiClient::lookup_cusip(std::string_view cusip)
{
    return extract_first_data(map(std::vector{MappingJob{.idType = IdType::ID_CUSIP, .idValue = std::string(cusip)}}));
}

Result<std::vector<Instrument>> OpenFigiClient::lookup_sedol(std::string_view sedol)
{
    return extract_first_data(map(std::vector{MappingJob{.idType = IdType::ID_SEDOL, .idValue = std::string(sedol)}}));
}

// ─── CSV Export (RFC 4180 compliant) ────────────────────────────────────────

namespace
{
std::string csv_escape(std::string_view field)
{
    bool needs_quoting = field.find_first_of(",\"\r\n") != std::string_view::npos;
    if (!needs_quoting)
        return std::string(field);

    std::string out;
    out.reserve(field.size() + 4);
    out.push_back('"');
    for (char c : field)
    {
        if (c == '"')
            out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

constexpr const char* INSTRUMENT_HEADER = "figi,name,ticker,exchCode,securityType,marketSector,"
    "securityType2,securityDescription,compositeFIGI,shareClassFIGI";

std::string instrument_csv_row(const Instrument& inst)
{
    return csv_escape(inst.figi) + "," + csv_escape(inst.name) + "," + csv_escape(inst.ticker) + "," +
           csv_escape(inst.exchCode) + "," + csv_escape(inst.securityType) + "," + csv_escape(inst.marketSector) + "," +
           csv_escape(inst.securityType2) + "," + csv_escape(inst.securityDescription) + "," +
           csv_escape(inst.compositeFIGI) + "," + csv_escape(inst.shareClassFIGI);
}
} // namespace

std::string export_csv(const std::vector<MappingResult>& results, bool include_header)
{
    std::string out;

    if (include_header)
    {
        out += "idType,idValue,status,";
        out += INSTRUMENT_HEADER;
        out += "\n";
    }

    for (const auto& mr : results)
    {
        std::string prefix;
        if (mr.original_request.contains("idType"))
            prefix += csv_escape(mr.original_request["idType"].get<std::string>());
        prefix += ",";
        if (mr.original_request.contains("idValue"))
            prefix += csv_escape(mr.original_request["idValue"].get<std::string>());
        prefix += ",";

        if (mr.has_data())
        {
            for (const auto& inst : mr.data())
            {
                out += prefix + "ok," + instrument_csv_row(inst) + "\n";
            }
        }
        else if (mr.has_warning())
        {
            out += prefix + csv_escape(mr.warning()) + ",,,,,,,,,,\n";
        }
        else
        {
            out += prefix + csv_escape("error: " + mr.error()) + ",,,,,,,,,,\n";
        }
    }
    return out;
}

std::string export_csv(const std::vector<Instrument>& instruments, bool include_header)
{
    std::string out;
    if (include_header)
    {
        out += INSTRUMENT_HEADER;
        out += "\n";
    }
    for (const auto& inst : instruments)
    {
        out += instrument_csv_row(inst) + "\n";
    }
    return out;
}

// ─── JSON export ────────────────────────────────────────────────────────────

nlohmann::json export_json(const std::vector<MappingResult>& results)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& mr : results)
    {
        nlohmann::json entry;
        entry["request"] = mr.original_request;

        if (mr.has_data())
        {
            nlohmann::json data_arr = nlohmann::json::array();
            for (const auto& inst : mr.data())
            {
                data_arr.push_back({{"figi", inst.figi}, {"name", inst.name}, {"ticker", inst.ticker},
                                    {"exchCode", inst.exchCode}, {"securityType", inst.securityType},
                                    {"marketSector", inst.marketSector}, {"securityType2", inst.securityType2},
                                    {"securityDescription", inst.securityDescription},
                                    {"compositeFIGI", inst.compositeFIGI}, {"shareClassFIGI", inst.shareClassFIGI},});
            }
            entry["data"] = std::move(data_arr);
        }
        else if (mr.has_warning())
        {
            entry["warning"] = mr.warning();
        }
        else
        {
            entry["error"] = mr.error();
        }

        arr.push_back(std::move(entry));
    }
    return arr;
}

nlohmann::json export_json(const std::vector<Instrument>& instruments)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& inst : instruments)
    {
        arr.push_back({{"figi", inst.figi}, {"name", inst.name}, {"ticker", inst.ticker}, {"exchCode", inst.exchCode},
                       {"securityType", inst.securityType}, {"marketSector", inst.marketSector},
                       {"securityType2", inst.securityType2}, {"securityDescription", inst.securityDescription},
                       {"compositeFIGI", inst.compositeFIGI}, {"shareClassFIGI", inst.shareClassFIGI},});
    }
    return arr;
}

} // namespace figi
