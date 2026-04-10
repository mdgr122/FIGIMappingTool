# FIGIMappingTool

A Windows desktop application and C++23 library for working with [FIGI](https://www.openfigi.com/) (Financial Instrument Global Identifier) data via the [OpenFIGI API v3](https://www.openfigi.com/api/documentation). Map identifiers to FIGIs, search instruments by keyword, or filter the entire universe by criteria — all from a native GUI or programmatically.

![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-23-brightgreen)
![License](https://img.shields.io/badge/license-MIT-green)

---

## Table of Contents

- [Features](#features)
- [GUI Application](#gui-application)
  - [Modes](#modes)
  - [Filter Fields](#filter-fields)
  - [Input File Formats](#input-file-formats)
  - [Output Formats](#output-formats)
- [When to Use Each Mode](#when-to-use-each-mode)
- [Input File Reference](#input-file-reference)
  - [TXT Format](#txt-format)
  - [CSV Format](#csv-format)
  - [How GUI Filters Interact with File Data](#how-gui-filters-interact-with-file-data)
- [Identifier Auto-Detection](#identifier-auto-detection)
- [Ticker Normalization](#ticker-normalization)
- [OpenFigiClient Library](#openfigiclient-library)
  - [Quick Lookups](#quick-lookups)
  - [Batch Mapping](#batch-mapping)
  - [Search (Keyword)](#search-keyword)
  - [Filter (Browse)](#filter-browse)
  - [Enum Values](#enum-values)
  - [Interval Filters](#interval-filters)
  - [Error Handling](#error-handling)
  - [Rate Limiting](#rate-limiting)
  - [CSV and JSON Export](#csv-and-json-export)
- [Building](#building)
- [Configuration](#configuration)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Supported Identifier Types](#supported-identifier-types)
- [License](#license)

---

## Features

- **Three operation modes** — Map File, Search, and Filter — covering all OpenFIGI v3 endpoints.
- **Full filter support in the GUI** — exchange code, MIC code, currency, market sector, security type, option type, state code, strike/coupon intervals, expiration/maturity date ranges, and an "include unlisted" toggle.
- **Two input file formats** — simple `.txt` (one identifier per line) or `.csv` with headers for per-row filter granularity.
- **Auto-detects identifier types** — ISIN, SEDOL, CUSIP, FIGI, and ticker patterns are recognized automatically, or specify explicitly in CSV.
- **GUI filters as defaults** — filter fields in the GUI apply to every mapping job as defaults; per-row values from your file take precedence.
- **Pagination** — Search and Filter results paginate automatically with a "Next Page" button that appends results.
- **Batch processing** — mapping jobs are automatically chunked into API-legal batch sizes (100 with key, 10 without).
- **Proper rate limiting** — sliding-window limiter syncs with server `ratelimit-remaining` / `ratelimit-reset` headers.
- **Automatic retry** — exponential backoff on HTTP 429, 500, and 503.
- **Thread-safe** — all shared state is mutex-protected; worker threads use `std::jthread` with automatic cleanup.
- **RFC 4180 CSV export** — correctly escapes commas, quotes, and newlines in instrument names.
- **v3 warning/error distinction** — "No identifier found" warnings are cleanly separated from server errors.

---

## GUI Application

The application window is organized top-to-bottom: file paths, mode selection, filter fields, action button, and status.

### Modes

Three radio buttons at the top of the filter area select the operation mode:

**Map File** — Load a `.txt` or `.csv` file of identifiers and resolve each one to its FIGI via the `/v3/mapping` endpoint. Filter fields in the GUI apply as global defaults to every job; per-row values from the file take priority. This is the mode for batch identifier resolution.

**Search** — Enter keywords in the Query field and hit SEARCH. Uses the `/v3/search` endpoint to find instruments by name or description. All filter fields are available to narrow results. Results come back in relevance order. When more pages are available, a "Next Page >>" button appears to load additional results.

**Filter** — Browse instruments by criteria via the `/v3/filter` endpoint. The Query field is optional — you can filter purely by exchange, sector, security type, etc. Results come back sorted alphabetically by FIGI with a total count. Pagination works the same as Search.

### Filter Fields

Every filter supported by the OpenFIGI v3 API is exposed in the GUI. All fields use placeholder text to indicate their purpose and expected format:

| Field | Placeholder | Format | Example |
|-------|-------------|--------|---------|
| ExchCode | `ExchCode (US)` | 2-letter exchange code | `US`, `LN`, `EU` |
| Currency | `Currency (USD)` | 3-letter currency code | `USD`, `EUR`, `GBP` |
| MIC Code | `MIC (XNAS)` | 4-letter ISO MIC | `XNAS`, `XNYS`, `XLON` |
| Option Type | `Call or Put` | `Call` or `Put` | `Call` |
| State Code | `State` | U.S. state code | `NY`, `CA` |
| Market Sector | `Market Sector (Equity)` | Sector name | `Equity`, `Corp`, `Govt` |
| Security Type | `Security Type` | Type name | `Common Stock`, `Equity Option` |
| Security Type 2 | `Security Type 2` | Secondary type | `Option`, `Corp`, `Pool` |
| Incl. Unlisted | Checkbox | Checked = include | |
| Strike | `min` / `max` | Decimal numbers | `100` / `200` |
| Coupon | `min` / `max` | Decimal numbers | `2.5` / `5.0` |
| Expiry | `YYYY-MM-DD` / `YYYY-MM-DD` | Date range (max 1 year) | `2024-01-01` / `2024-12-31` |
| Maturity | `YYYY-MM-DD` / `YYYY-MM-DD` | Date range (max 1 year) | `2026-01-01` / `2026-12-31` |

Leave any field empty to omit it from the request. Note that `exchCode` and `micCode` are mutually exclusive per the API — if both are provided, the API will reject the request.

### Input File Formats

In **Map File** mode, the file open dialog accepts both `.txt` and `.csv` files. The tool detects the format by extension:

- `.csv` files are parsed as CSV with a header row (see [CSV Format](#csv-format)).
- All other files (`.txt`, etc.) are parsed as one-identifier-per-line (see [TXT Format](#txt-format)).
- If a `.csv` file does not have a recognized header (no `idValue` column), it falls back to the TXT parser.

### Output Formats

The output format is determined by the save path extension:

| Extension | Format | Description |
|-----------|--------|-------------|
| `.csv` | RFC 4180 CSV | For Map mode: columns include `idType`, `idValue`, `status`, and all instrument fields. For Search/Filter: instrument fields only. All values properly escaped. |
| `.json` (or any other) | Pretty-printed JSON | For Map mode: array of objects with `request`, `data`/`warning`/`error`. For Search/Filter: array of instrument objects. |

---

## When to Use Each Mode

| I want to... | Mode | Why |
|-------------|------|-----|
| Resolve a list of ISINs/CUSIPs/tickers to FIGIs | **Map File** | Mapping is designed for known-identifier → FIGI resolution. Batches automatically. |
| Resolve identifiers with per-row exchange/currency filters | **Map File** + CSV input | CSV columns let you set different filters per identifier. |
| Resolve identifiers with a single global filter | **Map File** + GUI filters | Set the filter once in the GUI; it applies to every row. |
| Find instruments by company name | **Search** | Keyword search finds instruments by name/description. |
| Find all IBM options on US exchanges expiring in 2025 | **Search** | Query "IBM" + exchCode "US" + securityType2 "Option" + expiry range. |
| Browse all equity instruments on a specific exchange | **Filter** | No query needed — just set exchCode and marketSecDes. |
| Count how many instruments match certain criteria | **Filter** | The status bar shows the total count from the API. |
| List valid exchange codes / currencies / sectors | Library API | `client.get_enum_values("exchCode")` — not yet in the GUI. |

---

## Input File Reference

### TXT Format

One identifier per line. Each line optionally has a space-separated context token:

```
AAPL
IBM US
MSFT XNAS
US0378331005
BBG000B9XRY4
B03MM40
459200101 USD
SPX Index
```

The format per line is:

```
<identifier> [context]
```

**Identifier** is auto-detected (see [Identifier Auto-Detection](#identifier-auto-detection)).

**Context** is interpreted by length:

| Length | Interpreted As | Example |
|--------|---------------|---------|
| 2 chars | `exchCode` | `US`, `LN` |
| 3 chars | `currency` | `USD`, `EUR` |
| 4 chars | `micCode` | `XNAS`, `XNYS` |
| `Index` (literal) | Appended to identifier | `SPX Index` |

Empty lines are skipped. Unrecognizable identifiers are silently skipped.

### CSV Format

A CSV file with a header row. The first row defines columns; subsequent rows are data. The only required column is the identifier value — everything else is optional:

```csv
idValue,idType,exchCode,currency,securityType2
AAPL,TICKER,US,,
US0378331005,ID_ISIN,,,
IBM,TICKER,,USD,Common Stock
BBG000BLNNH6,ID_BB_GLOBAL,,,
TSLA 10 C100,BASE_TICKER,,,Option
```

#### Recognized Columns

Headers are matched case-insensitively with multiple aliases accepted:

| Column | Aliases | Required | Description |
|--------|---------|:--------:|-------------|
| `idValue` | `id_value`, `value`, `identifier` | **Yes** | The identifier to look up. |
| `idType` | `id_type`, `type` | No | Explicit identifier type (e.g., `TICKER`, `ID_ISIN`). If omitted, auto-detected. |
| `exchCode` | `exch_code`, `exchange` | No | Exchange code filter for this row. |
| `micCode` | `mic_code`, `mic` | No | MIC code filter for this row. |
| `currency` | `ccy` | No | Currency filter for this row. |
| `marketSecDes` | `market_sec_des`, `sector`, `marketSector` | No | Market sector for this row. |
| `securityType` | `security_type`, `secType` | No | Security type for this row. |
| `securityType2` | `security_type2`, `secType2` | No | Secondary security type for this row. |
| `optionType` | `option_type` | No | `Call` or `Put` for this row. |
| `stateCode` | `state_code`, `state` | No | U.S. state code for this row. |

#### Fallback Behavior

If the first row of a `.csv` file does not contain a recognized `idValue` column (or any of its aliases), the parser falls back to the TXT parser and treats each line as a simple identifier.

#### Full-Granularity Example

A CSV where each row has different filters — mix of equities, options, and bonds:

```csv
idValue,idType,exchCode,currency,marketSecDes,securityType2
AAPL,TICKER,US,,,Common Stock
TSLA 10 C100,BASE_TICKER,,,Equity,Option
IBM,TICKER,,USD,Corp,Corp
FG,BASE_TICKER,,,Mtge,Pool
US0378331005,ID_ISIN,,,,
BBG000BLNNH6,,,,, 
```

Note that `idType` can be left empty for any row — the tool will auto-detect. And any filter column can be left empty to omit that filter for that specific row.

### How GUI Filters Interact with File Data

In Map File mode, the GUI filter fields act as **defaults** that are applied to every mapping job. Per-row values from the file **take precedence**.

The rule is: if a job already has a value for a field (from the file), the GUI value is ignored for that field. If the job does not have a value, the GUI value is applied.

**Example:**

GUI fields set: `exchCode = US`, `currency = USD`

Input file (TXT):
```
AAPL
IBM LN
MSFT XNAS
```

Resulting jobs:
- `AAPL` → exchCode=`US` (from GUI), currency=`USD` (from GUI)
- `IBM` → exchCode=`LN` (from file, overrides GUI), currency=`USD` (from GUI)
- `MSFT` → micCode=`XNAS` (from file), currency=`USD` (from GUI) — note: `exchCode=US` from GUI is still applied, but `exchCode` and `micCode` are mutually exclusive, so validate your inputs

For CSV files, the same principle applies: CSV column values override GUI defaults, empty CSV cells fall through to GUI defaults.

---

## Identifier Auto-Detection

When no `idType` is specified (TXT format, or CSV with empty `idType` column), the tool auto-detects using pattern matching. Detection is applied in this order — the first match wins:

| Pattern | Detected As | Example |
|---------|-------------|---------|
| `BBG` + 9 alphanumeric chars (12 total) | `ID_BB_GLOBAL` (FIGI) | `BBG000BLNNH6` |
| 2 uppercase letters + 9 alphanumeric + 1 digit (12 total, not `BBG`) | `ID_ISIN` | `US0378331005` |
| 6 chars from SEDOL character set + 1 check digit (7 total) | `ID_SEDOL` | `B03MM40` |
| 3 digits + 6 alphanumeric (9 total) | `ID_CUSIP` | `459200101` |
| Alphanumeric with optional `.`/`-`/`/` suffix, optional ` Index` | `TICKER` | `AAPL`, `BRK.B`, `SPX Index` |

**Ambiguity note**: Some identifiers overlap patterns. A 9-character alphanumeric string could be CUSIP, CINS, or Common Code — the auto-detector defaults to CUSIP. Use the CSV `idType` column to override when needed.

---

## Ticker Normalization

Tickers go through a dot-notation normalization step before being sent to the API:

| Input | Transformed To | Rule |
|-------|---------------|------|
| `BRK.B` | `BRK/B` | Single character after `.` → replace `.` with `/` |
| `BRK.AB` | `BRK-A` | Multiple characters after `.` → replace with `-` + first character |
| `IBM.` | `IBM` | Trailing `.` → remove |
| `SPX Index` | `SPX Index` | ` Index` suffix preserved as-is |

This matches the conventions expected by the OpenFIGI API for Bloomberg-style ticker notation.

---

## OpenFigiClient Library

The `figi::OpenFigiClient` class is a standalone, thread-safe C++23 API client. It can be used independently of the GUI — include `OpenFigiClient.h`, link against CPR and nlohmann/json, and you have full programmatic access to all endpoints.

### Quick Lookups

```cpp
figi::OpenFigiClient client({.api_key = "your-key"});

auto r = client.lookup_figi("BBG000BLNNH6");       // by FIGI
auto r = client.lookup_ticker("IBM", "US");          // by ticker + exchange
auto r = client.lookup_isin("US0378331005");         // by ISIN
auto r = client.lookup_cusip("459200101");           // by CUSIP
auto r = client.lookup_sedol("B03MM40");             // by SEDOL
```

Each returns `Result<std::vector<Instrument>>`. A single identifier can map to multiple instruments (same stock on different exchanges).

### Batch Mapping

```cpp
std::vector<figi::MappingJob> jobs = {
    {.idType = figi::IdType::ID_ISIN,  .idValue = "US0378331005"},
    {.idType = figi::IdType::TICKER,   .idValue = "MSFT", .exchCode = "US"},
    {.idType = figi::IdType::ID_CUSIP, .idValue = "459200101",  .currency = "USD"},
    {
        .idType = figi::IdType::BASE_TICKER,
        .idValue = "TSLA 10 C100",
        .securityType2 = "Option",
        .expiration = figi::Interval<std::string>{"2024-01-01", "2024-12-31"},
    },
};

auto results = client.map(std::move(jobs));
if (results) {
    for (const auto& mr : *results) {
        if (mr.has_data())         // mr.data() → vector<Instrument>
        else if (mr.has_warning()) // mr.warning() → "No identifier found."
        else                       // mr.error() → server error message
    }
}
```

Jobs are auto-batched (100 per request with key, 10 without) with rate limiting between batches.

### Search (Keyword)

```cpp
// Find Tesla instruments on US exchanges
auto result = client.search({.query = "Tesla", .exchCode = "US"});

// With full filters
auto result = client.search({
    .query = "IBM",
    .marketSecDes = "Corp",
    .securityType2 = "Corp",
    .maturity = figi::Interval<std::string>{"2026-01-01", std::nullopt},
});

// Auto-paginate all results
auto all = client.search_all({.query = "Tesla", .marketSecDes = "Equity"});
```

### Filter (Browse)

```cpp
// Browse all US equity options with strike between 100 and 200
auto result = client.filter({
    .exchCode = "US",
    .securityType2 = "Option",
    .strike = figi::Interval<double>{100.0, 200.0},
});
// result->total = total matching instruments
// result->data  = first page

// Auto-paginate
auto all = client.filter_all({.exchCode = "US"}, /*max_pages=*/50);
```

### Enum Values

Query the API for valid values of enum-like fields:

```cpp
auto sectors    = client.get_enum_values("marketSecDes");
// → {"Comdty", "Corp", "Curncy", "Equity", "Govt", "Index", "M-Mkt", "Mtge", "Muni", "Pfd"}

auto exchanges  = client.get_enum_values("exchCode");
auto currencies = client.get_enum_values("currency");
auto id_types   = client.get_enum_values("idType");
auto sec_types  = client.get_enum_values("securityType2");
auto mic_codes  = client.get_enum_values("micCode");
```

### Interval Filters

Strike, coupon, contract size, expiration, and maturity use `Interval<T>`. Each bound is optional (`std::nullopt` = unbounded):

```cpp
figi::Interval<double>{100.0, 200.0}            // 100 ≤ x ≤ 200
figi::Interval<double>{100.0, std::nullopt}      // x ≥ 100
figi::Interval<double>{std::nullopt, 200.0}      // x ≤ 200
figi::Interval<std::string>{"2024-01-01", "2024-12-31"}  // date range
figi::Interval<std::string>{"2026-11-01", std::nullopt}  // from date onward (API caps at +1 year)
```

### Error Handling

All methods return `figi::Result<T>` (`std::expected<T, figi::Error>`):

```cpp
auto result = client.lookup_ticker("IBM");
if (!result) {
    const auto& err = result.error();
    err.code;            // ErrorCode enum (BadRequest, Unauthorized, TooManyRequests, etc.)
    err.message;         // human-readable
    err.detail;          // raw response body
    err.is_retryable();  // true for 429, 500, 503
}
```

Mapping results have three-way outcomes per job:

| Outcome | Meaning | Access |
|---------|---------|--------|
| `has_data()` | Instruments found | `mr.data()` → `vector<Instrument>` |
| `has_warning()` | No match (normal, not an error) | `mr.warning()` → `"No identifier found."` |
| `has_error()` | Server error for this job | `mr.error()` → error message |

This matches the v3 API's `"warning"` vs `"error"` distinction.

### Rate Limiting

Handled automatically:

| Endpoint | Without API Key | With API Key |
|----------|----------------|--------------|
| Mapping | 25 requests / 60s | 25 requests / 6s |
| Search & Filter | 5 requests / 60s | 20 requests / 60s |
| Max jobs per mapping request | 10 | 100 |

The `RateLimiter` uses a sliding window, sleeps instead of busy-waiting, and syncs with server rate-limit headers after every response. Limiters auto-reconfigure when the API key changes.

Transient failures (429, 500, 503) are retried with exponential backoff up to `max_retries` (default 3), respecting the `ratelimit-reset` header.

### CSV and JSON Export

```cpp
// From mapping results
std::string csv  = figi::export_csv(mapping_results);
nlohmann::json j = figi::export_json(mapping_results);

// From search/filter instrument lists
std::string csv  = figi::export_csv(instruments);
nlohmann::json j = figi::export_json(instruments);
```

---

## Building

### Requirements

- **Windows 10/11 x64**
- **Visual Studio 2022** (v17.7+) for C++23 `std::expected` support
- **CMake 3.25+**

### Steps

```bash
git clone https://github.com/mdgr122/FIGIMappingTool.git
cd FIGIMappingTool
cmake --preset x64-debug
cmake --build out/build/x64-debug
```

Or open the folder in Visual Studio — it detects `CMakePresets.json` and configures automatically.

### Compiler Feature Requirements

| Feature | Minimum MSVC |
|---------|-------------|
| `std::expected` | 17.7 |
| `std::jthread` / `std::stop_token` | 17.2 |
| `std::format` | 17.1 |
| Designated initializers | 17.0 |

---

## Configuration

### API Key

The application works without an API key but with reduced rate limits and smaller batch sizes. To get a key:

1. [Sign up](https://www.openfigi.com/user/signup) for a free OpenFIGI account.
2. [View your API key](https://www.openfigi.com/user/profile/api-key) after logging in.
3. Enter the key in the API key field at the bottom of the application window.

The key is held in memory only — never stored on disk or in source code.

### Programmatic Configuration

```cpp
figi::OpenFigiClient client({
    .api_key     = "your-key",
    .max_retries = 3,
    .on_progress = [](size_t done, size_t total) { /* ... */ },
});

client.set_api_key("new-key");  // thread-safe, reconfigures rate limiters
```

---

## Project Structure

```
FIGIMappingTool/
├── CMakeLists.txt
├── CMakePresets.json
├── CHANGELOG.md
├── README.md
│
└── FIGIMappingTool/
    ├── main.cpp                        # Win32 entry point
    ├── FIGIMappingTool.rc              # Windows resource file
    ├── FIGIMappingTool.manifest        # Application manifest
    │
    ├── core/
    │   ├── OpenFigiClient.h            # API client — all endpoints, types, export
    │   └── OpenFigiClient.cpp          # API client implementation
    |
    ├── states/
    │   ├── BaseWindow.h                # CRTP Win32 window base class
    │   ├── WindowState.h               # Main window — all modes, controls, threads
    │   ├── WindowState.cpp             # GUI logic, input parsing, worker threads
    │   ├── FileState.h                 # File I/O (open/save dialogs, read/write)
    │   └── FileState.cpp               # File I/O implementation
    │
    ├── utilities/
    │   └──Utils.h                      # UTF-8 ↔ UTF-16 conversion
    │
    └── libs/
        ├── json/json.hpp               # nlohmann/json (header-only)
        └── cpr/                        # CPR HTTP library
```

---

## Dependencies

| Library | Purpose | Included |
|---------|---------|----------|
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing/serialization | Yes |
| [CPR](https://github.com/libcpr/cpr) | HTTP client (wraps libcurl) | Yes |
| Windows SDK 10.0+ | Win32 API, Common Controls | System |

---

## Supported Identifier Types

| IdType | Description | Auto-Detected |
|--------|-------------|:---:|
| `ID_ISIN` | International Securities Identification Number (12 chars) | ✔ |
| `ID_SEDOL` | Stock Exchange Daily Official List (7 chars) | ✔ |
| `ID_CUSIP` | Committee on Uniform Securities Identification Procedures (9 chars) | ✔ |
| `ID_BB_GLOBAL` | Financial Instrument Global Identifier / FIGI (12 chars, `BBG` prefix) | ✔ |
| `TICKER` | Exchange ticker symbol | ✔ |
| `COMPOSITE_ID_BB_GLOBAL` | Composite FIGI | |
| `ID_BB_GLOBAL_SHARE_CLASS_LEVEL` | Share Class FIGI | |
| `ID_BB_UNIQUE` | Legacy Bloomberg unique identifier | |
| `ID_COMMON` | Common Code (9 digits) | |
| `ID_WERTPAPIER` | German WKN (6 chars) | |
| `ID_CINS` | CUSIP International Numbering System | |
| `ID_BB` | Legacy Bloomberg identifier | |
| `ID_BB_8_CHR` | Legacy Bloomberg identifier (8 chars) | |
| `ID_TRACE` | FINRA TRACE identifier | |
| `ID_ITALY` | Italian identifier (5–6 digits) | |
| `ID_EXCH_SYMBOL` | Exchange-specific symbol | |
| `ID_FULL_EXCHANGE_SYMBOL` | Full exchange symbol (futures/options/indices) | |
| `ID_BB_SEC_NUM_DES` | Security ID Number Description | |
| `BASE_TICKER` | Indistinct ticker (may need additional filters) | |
| `ID_CUSIP_8_CHR` | CUSIP (8 chars only) | |
| `OCC_SYMBOL` | OCC standardized option symbol (21 chars) | |
| `UNIQUE_ID_FUT_OPT` | Bloomberg futures/options identifier | |
| `OPRA_SYMBOL` | OPRA standardized option symbol | |
| `TRADING_SYSTEM_IDENTIFIER` | Trading system-specific identifier | |
| `ID_SHORT_CODE` | Asian fixed income short code | |
| `VENDOR_INDEX_CODE` | Index provider code | |

Types not marked as auto-detected can be used by specifying `idType` explicitly in a CSV input file or via the library API.

---

## License

MIT License — see [LICENSE](LICENSE) for details.

### OpenFIGI API License

The OpenFIGI API is provided by Bloomberg Finance L.P. under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0). By using this tool, you agree to comply with the OpenFIGI API [Terms of Use](https://www.openfigi.com/api#terms) and [Terms of Service](https://www.openfigi.com/docs/terms-of-service).

---

*Please ensure that you have read and understood the OpenFIGI API terms and conditions. It is your responsibility to comply with all applicable licensing requirements.*

## Acknowledgements

- **OpenFIGI API**: This project utilizes the OpenFIGI API provided by Bloomberg Finance L.P.

## Disclaimer

This tool is an independent project and is not affiliated with, endorsed by, or associated with Bloomberg Finance L.P. or OpenFIGI. The use of the OpenFIGI API is subject to their [Terms of Use](https://www.openfigi.com/api#terms).