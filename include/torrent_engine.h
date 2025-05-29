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
#include <mutex>
#include <atomic>
#include <fmt/format.h>
#include "spdlog/spdlog.h"

namespace puji
{
    // 进度条管理器单例
    struct ProgressManager
    {
        static ProgressManager &getInstance()
        {
            static ProgressManager instance;
            return instance;
        }

        ProgressManager(const ProgressManager &) = delete;
        ProgressManager &operator=(const ProgressManager &) = delete;
        ProgressManager(ProgressManager &&) = delete;
        ProgressManager &operator=(ProgressManager &&) = delete;

        indicators::ProgressBar &get_progress_bar(std::size_t index)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return progress_bars_[index];
        }

        std::size_t add_progress_bar()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            progress_bars_.push_back(std::make_unique<indicators::ProgressBar>(
                indicators::option::BarWidth{50},
                indicators::option::Start{"["},
                indicators::option::Fill{"="},
                indicators::option::Lead{">"},
                indicators::option::Remainder{" "},
                indicators::option::End{"]"},
                indicators::option::ForegroundColor{indicators::Color::cyan},
                indicators::option::ShowElapsedTime{true},
                indicators::option::ShowRemainingTime{true},
                indicators::option::FontStyles{
                    std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}));
            return progress_bar_count_++;
        }

    private:
        ProgressManager() = default;
        ~ProgressManager() = default;

        indicators::DynamicProgress<indicators::ProgressBar> progress_bars_;
        std::atomic<std::size_t> progress_bar_count_{0};
        mutable std::mutex mutex_;
    };

    struct TaskManager
    {
    public:
        struct Task
        {
            libtorrent::torrent_handle handle;
            libtorrent::add_torrent_params params;
            libtorrent::torrent_status status;
            std::vector<libtorrent::alert *> alerts;
            std::size_t bar_index;
            std::atomic<bool> is_finished{false};

            Task();
            Task(const Task &) = delete;
            Task &operator=(const Task &) = delete;
            Task(Task &&other) noexcept;
            Task &operator=(Task &&other) noexcept;
            ~Task() = default;
        };

        static std::vector<std::string> trackers; // 声明为静态成员变量

        TaskManager();
        ~TaskManager();
        TaskManager(const TaskManager &) = delete;
        TaskManager &operator=(const TaskManager &) = delete;
        TaskManager(TaskManager &&) = delete;
        TaskManager &operator=(TaskManager &&) = delete;

        void add_task(const std::string &src, const std::string &save_path);
        void remove_finished_tasks();
        bool empty() const;
        std::vector<Task> &get_tasks();
        void check_torrent_status(Task &task);
        void check_torrent_polling();
        void async_bitorrent_download();

    private:
        std::unique_ptr<libtorrent::session> session_;
        std::vector<Task> tasks_;
        mutable std::mutex mutex_;

        void configure_session();
        bool is_magnet_uri(const std::string &str)
        {
            return str.find("magnet:?xt=urn:btih:") == 0;
        }
    };

    struct TorrentDownloader
    {
    public:
        TorrentDownloader(const std::string &src,
                          const std::string &save_path = "./download")
            : task_manager_()
        {
            task_manager_.add_task(src, save_path);
        }

        TorrentDownloader(const std::vector<std::string> &src,
                          const std::string &save_path = "./download")
            : task_manager_()
        {
            for (const auto &s : src)
            {
                task_manager_.add_task(s, save_path);
            }
        }

        void wait()
        {
            task_manager_.check_torrent_polling();
        }

        void async_bitorrent_download()
        {
            task_manager_.async_bitorrent_download();
        }

    private:
        TaskManager task_manager_;
        bool is_magnet_uri(const std::string &str)
        {
            return str.find("magnet:?xt=urn:btih:") == 0;
        }
    };

} // namespace puji

#endif
