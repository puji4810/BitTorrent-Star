#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <memory>
#include <ranges>
#include <thread>
#include <vector>

struct task {
    libtorrent::session session;
    libtorrent::add_torrent_params params;
    libtorrent::torrent_status status;
    std::unique_ptr<indicators::ProgressBar> progress_bar;
    std::vector<libtorrent::alert *> alerts;

    task() {
        progress_bar = std::make_unique<indicators::ProgressBar>(
            indicators::option::BarWidth{50}, indicators::option::Start{"["},
            indicators::option::Fill{"="}, indicators::option::Lead{">"},
            indicators::option::Remainder{" "}, indicators::option::End{"]"},
            indicators::option::ForegroundColor{indicators::Color::cyan},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{
                std::vector<indicators::FontStyle>{indicators::FontStyle::bold}
            });
    }

    task(const task &) = delete;

    task(task &&) = default;

    task &operator=(const task &) = delete;

    task &operator=(task &&) = default;

    ~task() = default;
};

struct torrent_downloader {
public:
    std::vector<task> tasks;
    // libtorrent::session session;
    libtorrent::add_torrent_params params;
    // indicators::ProgressBar progress_bar;
    indicators::DynamicProgress<indicators::ProgressBar> bars;

    torrent_downloader(const std::string &src,
                       const std::string &save_path = "./download") {
        params.save_path = save_path;
        task tk;

        if (is_magnet_uri(src)) {
            params = libtorrent::parse_magnet_uri(src);
        } else {
            params.ti = std::make_shared<libtorrent::torrent_info>(src);
        }
        tk.params = params;
        tasks.emplace_back(std::move(tk));
    }

    torrent_downloader(const std::vector<std::string> &src,
                       const std::string &save_path = "./download") {
        params.save_path = save_path;
        for (const auto &s: src) {
            task tk;
            if (is_magnet_uri(s)) {
                params = libtorrent::parse_magnet_uri(s);
            } else {
                params.ti = std::make_shared<libtorrent::torrent_info>(s);
            }
            tk.params = std::move(params);
            tasks.emplace_back(std::move(tk));
        }
    }

    void wait() { check_torrent(); }

    void async_bitorrent_download();

    int bitorrent_download();

private:
    void set_session(lt::session &session);

    void check_torrent_helper(lt::session &session, lt::torrent_status &st,
                              std::size_t bars_index,
                              std::vector<lt::alert *> &alerts);

    void check_torrent();

    bool is_magnet_uri(const std::string &str) {
        return str.find("magnet:?xt=urn:btih:") == 0; // 判断是否以 "magnet:?xt=urn:btih:" 开头
    }
};
