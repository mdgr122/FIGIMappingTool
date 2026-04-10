#pragma once
// OpenFigiClient.h — C++23 OpenFIGI API client
// Supports /v3/mapping, /v3/search, /v3/filter, /v3/mapping/values/:key
// Thread-safe, rate-limit-aware, with proper error handling.

#include <chrono>
#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <json.hpp>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace figi
{

// ─── Error types ────────────────────────────────────────────────────────────

enum class ErrorCode
{
    BadRequest      = 400,
    Unauthorized    = 401,
    NotFound        = 404,
    PayloadTooLarge = 413,
    TooManyRequests = 429,
    ServerError     = 500,
    Unavailable     = 503,
    NetworkError    = 0,
    ParseError      = 1,
    InvalidInput    = 2, };

struct Error
{
    ErrorCode code;
    std::string message;
    std::string detail;

    [[nodiscard]] constexpr bool is_retryable() const noexcept
    {
        return code == ErrorCode::TooManyRequests || code == ErrorCode::ServerError || code == ErrorCode::Unavailable;
    }
};

template <typename T>
using Result = std::expected<T, Error>;

// ─── Domain types ───────────────────────────────────────────────────────────

enum class IdType
{
    ID_ISIN,
    ID_BB_UNIQUE,
    ID_SEDOL,
    ID_COMMON,
    ID_WERTPAPIER,
    ID_CUSIP,
    ID_CINS,
    ID_BB,
    ID_BB_8_CHR,
    ID_TRACE,
    ID_ITALY,
    ID_EXCH_SYMBOL,
    ID_FULL_EXCHANGE_SYMBOL,
    COMPOSITE_ID_BB_GLOBAL,
    ID_BB_GLOBAL_SHARE_CLASS_LEVEL,
    ID_BB_GLOBAL,
    ID_BB_SEC_NUM_DES,
    TICKER,
    BASE_TICKER,
    ID_CUSIP_8_CHR,
    OCC_SYMBOL,
    UNIQUE_ID_FUT_OPT,
    OPRA_SYMBOL,
    TRADING_SYSTEM_IDENTIFIER,
    ID_SHORT_CODE,
    VENDOR_INDEX_CODE, };

[[nodiscard]] constexpr std::string_view to_string(IdType t) noexcept;
[[nodiscard]] std::optional<IdType> parse_id_type(std::string_view s) noexcept;

struct Instrument
{
    std::string figi;
    std::string name;
    std::string ticker;
    std::string exchCode;
    std::string securityType;
    std::string marketSector;
    std::string securityType2;
    std::string securityDescription;
    std::string compositeFIGI;
    std::string shareClassFIGI;

    static Instrument from_json(const nlohmann::json& j);
};

struct MappingResult
{
    // index 0 = data, index 1 = warning string, index 2 = error string
    std::variant<std::vector<Instrument>, std::string, std::string> outcome;

    nlohmann::json original_request;

    [[nodiscard]] bool has_data() const { return outcome.index() == 0; }
    [[nodiscard]] bool has_warning() const { return outcome.index() == 1; }
    [[nodiscard]] bool has_error() const { return outcome.index() == 2; }

    [[nodiscard]] const auto& data() const { return std::get<0>(outcome); }
    [[nodiscard]] const auto& warning() const { return std::get<1>(outcome); }
    [[nodiscard]] const auto& error() const { return std::get<2>(outcome); }
};

// ─── Interval helper ────────────────────────────────────────────────────────

template <typename T>
struct Interval
{
    std::optional<T> lower;
    std::optional<T> upper;

    [[nodiscard]] nlohmann::json to_json() const
    {
        nlohmann::json arr = nlohmann::json::array();
        arr.push_back(lower ? nlohmann::json(*lower) : nlohmann::json(nullptr));
        arr.push_back(upper ? nlohmann::json(*upper) : nlohmann::json(nullptr));
        return arr;
    }
};

// ─── Mapping Job ────────────────────────────────────────────────────────────

struct MappingJob
{
    IdType idType;
    std::string idValue;

    std::optional<std::string> exchCode;
    std::optional<std::string> micCode;
    std::optional<std::string> currency;
    std::optional<std::string> marketSecDes;
    std::optional<std::string> securityType;
    std::optional<std::string> securityType2;
    std::optional<bool> includeUnlistedEquities;
    std::optional<std::string> optionType;
    std::optional<std::string> stateCode;

    std::optional<Interval<double>> strike;
    std::optional<Interval<double>> contractSize;
    std::optional<Interval<double>> coupon;
    std::optional<Interval<std::string>> expiration;
    std::optional<Interval<std::string>> maturity;

    [[nodiscard]] nlohmann::json to_json() const;
    [[nodiscard]] Result<void> validate() const;
};

// ─── Search / Filter Request ────────────────────────────────────────────────

struct SearchRequest
{
    std::optional<std::string> query;
    std::optional<std::string> start;

    std::optional<std::string> exchCode;
    std::optional<std::string> micCode;
    std::optional<std::string> currency;
    std::optional<std::string> marketSecDes;
    std::optional<std::string> securityType;
    std::optional<std::string> securityType2;
    std::optional<bool> includeUnlistedEquities;
    std::optional<std::string> optionType;
    std::optional<std::string> stateCode;

    std::optional<Interval<double>> strike;
    std::optional<Interval<double>> contractSize;
    std::optional<Interval<double>> coupon;
    std::optional<Interval<std::string>> expiration;
    std::optional<Interval<std::string>> maturity;

    [[nodiscard]] nlohmann::json to_json() const;
};

struct SearchResponse
{
    std::vector<Instrument> data;
    std::optional<std::string> next;
    std::optional<int64_t> total;
};

// ─── Rate Limiter ───────────────────────────────────────────────────────────

class RateLimiter
{
public:
    explicit RateLimiter(int max_requests, std::chrono::seconds window);

    void acquire();
    void update_from_headers(int remaining, std::chrono::seconds reset);
    void reconfigure(int max_requests, std::chrono::seconds window);

private:
    std::mutex m_mutex;
    int m_max;
    std::chrono::seconds m_window;
    std::vector<std::chrono::steady_clock::time_point> m_timestamps;
    std::optional<std::chrono::steady_clock::time_point> m_server_reset;
    std::optional<int> m_server_remaining;
};

// ─── Client ─────────────────────────────────────────────────────────────────

struct ClientConfig
{
    std::string api_key;
    int max_retries = 3;

    // Progress callback: (completed, total).  Called from worker thread.
    std::function<void(size_t, size_t)> on_progress;
};

class OpenFigiClient
{
public:
    explicit OpenFigiClient(ClientConfig config = { });

    // Update the API key at runtime (e.g. from GUI input).
    void set_api_key(std::string_view key);
    [[nodiscard]] std::string get_api_key() const;

    // ── Mapping ─────────────────────────────────────────────────────────
    [[nodiscard]] Result<std::vector<MappingResult>> map(std::vector<MappingJob> jobs);

    [[nodiscard]] Result<std::vector<MappingResult>>
    map(std::string_view id_value, std::optional<std::string_view> context = std::nullopt);

    // ── Search ──────────────────────────────────────────────────────────
    [[nodiscard]] Result<SearchResponse> search(SearchRequest req);
    [[nodiscard]] Result<std::vector<Instrument>> search_all(SearchRequest req, int max_pages = 150);

    // ── Filter ──────────────────────────────────────────────────────────
    [[nodiscard]] Result<SearchResponse> filter(SearchRequest req);
    [[nodiscard]] Result<std::vector<Instrument>> filter_all(SearchRequest req, int max_pages = 150);

    // ── Enum values ─────────────────────────────────────────────────────
    [[nodiscard]] Result<std::vector<std::string>> get_enum_values(std::string_view key);

    // ── Identifier helpers ──────────────────────────────────────────────
    [[nodiscard]] static std::optional<IdType> detect_id_type(std::string_view value);

    [[nodiscard]] Result<std::vector<Instrument>> lookup_figi(std::string_view figi);

    [[nodiscard]] Result<std::vector<Instrument>>
    lookup_ticker(std::string_view ticker, std::optional<std::string_view> exch_code = std::nullopt);

    [[nodiscard]] Result<std::vector<Instrument>> lookup_isin(std::string_view isin);

    [[nodiscard]] Result<std::vector<Instrument>> lookup_cusip(std::string_view cusip);

    [[nodiscard]] Result<std::vector<Instrument>> lookup_sedol(std::string_view sedol);

private:
    mutable std::mutex m_config_mutex;
    ClientConfig m_config;
    RateLimiter m_mapping_limiter;
    RateLimiter m_search_limiter;

    static constexpr std::string_view BASE_URL = "https://api.openfigi.com";

    struct HttpResponse
    {
        int status;
        std::string body;
        int ratelimit_remaining = -1;
        int ratelimit_reset = -1;
    };

    [[nodiscard]] Result<HttpResponse>
    post(std::string_view endpoint, const nlohmann::json& body, RateLimiter& limiter);

    [[nodiscard]] Result<HttpResponse> get(std::string_view endpoint, RateLimiter& limiter);

    [[nodiscard]] Result<HttpResponse>
    request_with_retry(std::function<Result<HttpResponse>()> do_request, RateLimiter& limiter);

    void update_limiter(RateLimiter& limiter, const HttpResponse& resp);
    static Result<nlohmann::json> parse_body(const HttpResponse& resp);

    std::string current_api_key() const;
    size_t batch_size() const;
};

// ─── CSV / JSON export ──────────────────────────────────────────────────────

[[nodiscard]] std::string export_csv(const std::vector<MappingResult>& results, bool include_header = true);

[[nodiscard]] std::string export_csv(const std::vector<Instrument>& instruments, bool include_header = true);

[[nodiscard]] nlohmann::json export_json(const std::vector<MappingResult>& results);

[[nodiscard]] nlohmann::json export_json(const std::vector<Instrument>& instruments);

} // namespace figi
