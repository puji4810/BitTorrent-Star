#include <chrono>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <memory>
#include <thread>
#include <vector>
struct task {
  libtorrent::session session;
  libtorrent::add_torrent_params params;
  libtorrent::torrent_status status;
  std::unique_ptr<indicators::ProgressBar> progress_bar;

  task() {
    progress_bar = std::make_unique<indicators::ProgressBar>(
        indicators::option::BarWidth{50}, indicators::option::Start{"["},
        indicators::option::Fill{"="}, indicators::option::Lead{">"},
        indicators::option::Remainder{" "}, indicators::option::End{"]"},
        indicators::option::ForegroundColor{indicators::Color::cyan},
        indicators::option::ShowElapsedTime{true},
        indicators::option::ShowRemainingTime{true},
        indicators::option::FontStyles{
            std::vector<indicators::FontStyle>{indicators::FontStyle::bold}});
  }
  task(const task &) = delete;            // 禁用拷贝构造函数
  task(task &&) = default;                // 启用移动构造函数
  task &operator=(const task &) = delete; // 禁用拷贝赋值运算符
  task &operator=(task &&) = default;     // 启用移动赋值运算符
  ~task() = default;
};

struct torrent_downloader {
public:
  std::vector<task> tasks;
  // libtorrent::session session;
  std::vector<libtorrent::alert *> alerts;
  libtorrent::add_torrent_params params;
  // indicators::ProgressBar progress_bar;

  torrent_downloader(const std::string &torrent_file,
                     const std::string &save_path = "./download") {
    params.save_path = save_path;
    params.ti = std::make_shared<libtorrent::torrent_info>(torrent_file);
    task tk;
    tk.params = params;
    tasks.emplace_back(std::move(tk));
  }

  torrent_downloader(const std::vector<std::string> &torrent_files,
                     const std::string &save_path = "./download") {
    params.save_path = save_path;
    for (const auto &torrent_file : torrent_files) {
      params.ti = std::make_shared<libtorrent::torrent_info>(torrent_file);
      task tk;
      tk.params = params;
      tasks.emplace_back(std::move(tk));
    }
  }

  void wait() { check_torrent(); }
  void async_bitorrent_download();
  int bitorrent_download();

private:
  void set_session(lt::session &session);
  void check_torrent_helper(lt::session &session, lt::torrent_status &st,
                            indicators::ProgressBar &progress_bar);
  void check_torrent();
};
