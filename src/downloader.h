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
  libtorrent::session session;
  libtorrent::torrent_handle handle;
  std::vector<libtorrent::alert *> alerts;

  torrent_downloader() {}

  void download(const std::string &torrent_file) {
    libtorrent::add_torrent_params params;
    params.save_path = "./";
    params.ti = std::make_shared<libtorrent::torrent_info>(torrent_file);
    handle = session.add_torrent(params);
  }

  void download(const libtorrent::add_torrent_params &params) {
    handle = session.add_torrent(params);
  }

  void download(const std::string &magnet_uri, const std::string &save_path) {
    libtorrent::add_torrent_params params;
    params.save_path = save_path;
    libtorrent::error_code ec;
    libtorrent::parse_magnet_uri(magnet_uri, params, ec);
    handle = session.add_torrent(params);
  }

  void download(const std::string &magnet_uri) { download(magnet_uri, "./"); }

  void wait() {
    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::vector<libtorrent::alert *> new_alerts;
      session.pop_alerts(&new_alerts);
      alerts.insert(alerts.end(), new_alerts.begin(), new_alerts.end());
      for (auto &alert : new_alerts) {
        if (auto *e =
                libtorrent::alert_cast<libtorrent::torrent_finished_alert>(
                    alert)) {
          return;
        }
      }
    }
  }

  void print_alerts() {
    for (auto &alert : alerts) {
      std::cout << alert->message() << std::endl;
    }
  }
};
