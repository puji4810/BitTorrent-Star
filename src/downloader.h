#include <chrono>
#include <iostream>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <thread>
#include <vector>

#include <indicators/progress_bar.hpp>

struct torrent_downloader {
  // std::vector<libtorrent::session> sessions;
  libtorrent::session session;
  libtorrent::torrent_handle handle;
  std::vector<libtorrent::alert *> alerts;
  libtorrent::add_torrent_params params;
  libtorrent::torrent_status st;
  indicators::ProgressBar progress_bar;

  torrent_downloader(const std::string &torrent_file,
                     const std::string &save_path = "./download")
      : progress_bar(
            indicators::option::BarWidth{50}, indicators::option::Start{"["},
            indicators::option::Fill{"="}, indicators::option::Lead{">"},
            indicators::option::Remainder{" "}, indicators::option::End{"]"},
            indicators::option::ForegroundColor{indicators::Color::cyan},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{
                indicators::FontStyle::bold}}) {
    params.save_path = save_path;
    params.ti = std::make_shared<libtorrent::torrent_info>(torrent_file);
  }

  void wait() { check_torrent(session); }

  void print_alerts() {
    for (auto &alert : alerts) {
      std::cout << alert->message() << std::endl;
    }
  }

  void set_session(lt::session &session);
  void check_torrent_helper(lt::session &session);
  void check_torrent(lt::session &session);
  void async_bitorrent_download();
  int bitorrent_download();
};
