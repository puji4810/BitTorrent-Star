#ifndef BITTORRENT_DOWNLOADER_H
#define BITTORRENT_DOWNLOADER_H

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
#include <fstream>
#include <fmt/format.h>
#include "spdlog/spdlog.h"

struct Task {
    std::unique_ptr<libtorrent::session> session;
    libtorrent::add_torrent_params params;
    libtorrent::torrent_status status;
    std::vector<libtorrent::alert *> alerts;
    std::size_t bar_index;
    bool is_finished = false;

    Task() {
        session = std::make_unique<libtorrent::session>();
    }

    Task(const Task &) = delete;

    Task &operator=(const Task &) = delete;

    Task(Task&& other) noexcept
        : session(std::move(other.session)),
          params(std::move(other.params)),
          status(std::move(other.status)),
          alerts(std::move(other.alerts)),
          bar_index(other.bar_index),
          is_finished(other.is_finished)
    {}

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            session = std::move(other.session);
            params = std::move(other.params);
            status = std::move(other.status);
            alerts = std::move(other.alerts);
            bar_index = other.bar_index;
            is_finished = other.is_finished;
        }
        return *this;
    }

    ~Task() = default;
};

// struct TaskManager {
//
// };

struct torrent_downloader {
public:
    static std::vector<std::string> trackers;
    static std::size_t bar_index;

    torrent_downloader(const std::string &src,
                       const std::string &save_path = "./download") {
        libtorrent::add_torrent_params params;
        Task tk;

        if (is_magnet_uri(src)) {
            params = libtorrent::parse_magnet_uri(src);
        } else {
            params.ti = std::make_shared<libtorrent::torrent_info>(src);
        }
        params.trackers = trackers;
        params.save_path = save_path;
        tk.params = params;
        tk.bar_index = bar_index++;
        bars.push_back(std::make_unique<indicators::ProgressBar>(
                indicators::option::BarWidth{50}, indicators::option::Start{"["},
                indicators::option::Fill{"="}, indicators::option::Lead{">"},
                indicators::option::Remainder{" "}, indicators::option::End{"]"},
                indicators::option::ForegroundColor{indicators::Color::cyan},
                indicators::option::ShowElapsedTime{true},
                indicators::option::ShowRemainingTime{true},
                indicators::option::FontStyles{
                    std::vector<indicators::FontStyle>{indicators::FontStyle::bold}
                }
            ));
        tasks.emplace_back(std::move(tk));
    }

    torrent_downloader(const std::vector<std::string> &src,
                       const std::string &save_path = "./download") {
        for (const auto &s: src) {
            libtorrent::add_torrent_params params;
            Task tk;
            if (is_magnet_uri(s)) {
                params = libtorrent::parse_magnet_uri(s);
            } else {
                params.ti = std::make_shared<libtorrent::torrent_info>(s);
            }
            params.trackers = trackers;
            params.save_path = save_path;
            tk.params = std::move(params);
            tk.bar_index = bar_index++;
            bars.push_back(std::make_unique<indicators::ProgressBar>(
                indicators::option::BarWidth{50}, indicators::option::Start{"["},
                indicators::option::Fill{"="}, indicators::option::Lead{">"},
                indicators::option::Remainder{" "}, indicators::option::End{"]"},
                indicators::option::ForegroundColor{indicators::Color::cyan},
                indicators::option::ShowElapsedTime{true},
                indicators::option::ShowRemainingTime{true},
                indicators::option::FontStyles{
                    std::vector<indicators::FontStyle>{indicators::FontStyle::bold}
                }
            ));
            tasks.emplace_back(std::move(tk));
        }
    }

    void wait() {
        check_torrent_polling();
    }

    void async_bitorrent_download();

    int bitorrent_download();

private:
    std::vector<Task> tasks;
    static indicators::DynamicProgress<indicators::ProgressBar> bars;

    void set_session(lt::session &session);

    void check_torrent_status(Task &);

    void check_torrent_polling();

    bool is_magnet_uri(const std::string &str) {
        return str.find("magnet:?xt=urn:btih:") == 0; // 判断是否以 "magnet:?xt=urn:btih:" 开头
    }
};

#endif
