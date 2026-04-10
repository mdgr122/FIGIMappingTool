#pragma once

#include "BaseWindow.h"
#include <Windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include "FileState.h"
#include "../core/OpenFigiClient.h"
#include "../utilities/Utils.h"
#include <memory>
#include <mutex>
#include <thread>
#include <optional>

// ─── Control IDs ────────────────────────────────────────────────────────────

#define ID_BUTTON_FILE_PATH   1001
#define ID_BUTTON_SAVE_PATH   1002
#define ID_BUTTON_SAVE        1003
#define ID_BUTTON_REQUEST     1004
#define ID_FILE_PATH          1005
#define ID_SAVE_PATH          1006
#define ID_STATIC_MSG         1007
#define ID_BUTTON_ABOUT       1008
#define ID_BUTTON_CLOSE       1009
#define ID_STATIC_ABOUT_MSG   1011
#define ID_EDIT_APIKEY        1012

#define ID_RADIO_MAP          1020
#define ID_RADIO_SEARCH       1021
#define ID_RADIO_FILTER       1022

#define ID_EDIT_QUERY         1030
#define ID_EDIT_EXCHCODE      1031
#define ID_EDIT_CURRENCY      1032
#define ID_EDIT_MICCODE       1033
#define ID_EDIT_OPTIONTYPE    1034
#define ID_EDIT_MARKETSECDES  1035
#define ID_EDIT_SECTYPE       1036
#define ID_EDIT_SECTYPE2      1037
#define ID_EDIT_STATECODE     1038
#define ID_CHECK_UNLISTED     1039
#define ID_EDIT_STRIKE_MIN    1040
#define ID_EDIT_STRIKE_MAX    1041
#define ID_EDIT_COUPON_MIN    1042
#define ID_EDIT_COUPON_MAX    1043
#define ID_EDIT_EXPIRY_MIN    1044
#define ID_EDIT_EXPIRY_MAX    1045
#define ID_EDIT_MATURITY_MIN  1046
#define ID_EDIT_MATURITY_MAX  1047
#define ID_BUTTON_NEXT_PAGE   1048

#define WM_APP_CHILD_CLOSED       (WM_APP + 1)
#define WM_MAKE_REQUEST_COMPLETE  (WM_APP + 2)
#define WM_MAKE_REQUEST_FAILED    (WM_APP + 3)
#define WM_SAVE_COMPLETE          (WM_USER + 3)

class FileState;

class WindowState : public BaseWindow<WindowState>
{
public:
    WindowState(HWND hParent, FileState& fileState, figi::OpenFigiClient* client);
    ~WindowState();

    PCWSTR ClassName() const override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    BOOL CreateParentWindow();
    BOOL CreateAboutWindow();

    HWND GetHWND() const { return m_hwndParent; }
    std::unique_ptr<WindowState> aboutWindow;

private:
    enum class OpMode
    {
        Map, Search, Filter
    };

    // ── GUI actions ─────────────────────────────────────────────────────
    void get_open_path();
    void get_save_path();
    void start_request_thread();
    void start_save_thread();
    void start_next_page_thread();
    void do_request();
    void do_save();
    void do_next_page();
    void on_mode_changed();
    void update_request_button_text();

    // ── Input parsing ───────────────────────────────────────────────────
    std::vector<figi::MappingJob> parse_txt_lines(const std::vector<std::string>& lines);
    std::vector<figi::MappingJob> parse_csv_lines(const std::vector<std::string>& lines);
    static void process_ticker(std::string& str);

    // ── GUI → filter structs ────────────────────────────────────────────
    std::string read_edit_text(HWND hwnd);
    void apply_gui_filters(figi::MappingJob& job);
    figi::SearchRequest build_search_request();
    std::optional<figi::Interval<double>> read_double_interval(HWND hMin, HWND hMax);
    std::optional<figi::Interval<std::string>> read_string_interval(HWND hMin, HWND hMax);

    // ── Helpers ─────────────────────────────────────────────────────────
    static std::vector<std::string> split_csv_line(const std::string& line);
    static bool is_csv_path(const std::string& path);
    int center_x(int w) const { return (PARENT_WINDOW_WIDTH - w) / 2; }

    // ── Thread-safe results ─────────────────────────────────────────────
    std::mutex m_results_mutex;
    OpMode m_mode = OpMode::Map;
    std::vector<figi::MappingResult> m_mapping_results;
    std::vector<figi::Instrument> m_search_results;
    std::optional<std::string> m_next_cursor;
    std::optional<int64_t> m_total_count;
    std::string m_error_msg;

    std::jthread m_request_thread;
    std::jthread m_save_thread;

    // ── Window handles ──────────────────────────────────────────────────
    HWND m_hwndParent;

    HWND hwndFilePath{ };
    HWND hwndFileButton{ };
    HWND hwndSavePath{ };
    HWND hwndSaveButton{ };
    HWND hwndSaveButton2{ };
    HWND hwndRadioMap{ };
    HWND hwndRadioSearch{ };
    HWND hwndRadioFilter{ };
    HWND hwndQuery{ };
    HWND hwndExchCode{ };
    HWND hwndCurrency{ };
    HWND hwndMicCode{ };
    HWND hwndOptType{ };
    HWND hwndStateCode{ };
    HWND hwndSector{ };
    HWND hwndSecType{ };
    HWND hwndSecType2{ };
    HWND hwndUnlisted{ };
    HWND hwndStrikeMin{ };
    HWND hwndStrikeMax{ };
    HWND hwndCouponMin{ };
    HWND hwndCouponMax{ };
    HWND hwndExpiryMin{ };
    HWND hwndExpiryMax{ };
    HWND hwndMaturityMin{ };
    HWND hwndMaturityMax{ };
    HWND hwndRequestButton{ };
    HWND hwndWaitingMsg{ };
    HWND hwndNextPageButton{ };
    HWND hwndAboutButton{ };
    HWND m_hwndAPIKey{ };
    HWND hwndCloseButton{ };
    HWND hwndAboutText{ };
    HWND m_hwndAboutWindow{ };

    // ── GDI ─────────────────────────────────────────────────────────────
    HBRUSH m_hbrBackground;
    HBRUSH hbrEditBackground{ };
    HFONT hFontAboutButtonText{ };
    HFONT hFontAboutText{ };
    HFONT hFontSmall{ };

    // ── Layout ──────────────────────────────────────────────────────────
    static constexpr int PARENT_WINDOW_WIDTH = 680;
    static constexpr int PARENT_WINDOW_HEIGHT = 510;
    int nWidth{ };
    int nHeight{ };

    std::string m_open_path;
    std::string m_save_path;
    std::wstring m_apikey;

    FileState& m_fileState;
    figi::OpenFigiClient* m_client;
};
