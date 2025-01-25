#include "downloader.h"

void torrent_downloader::set_session(lt::session &session) {
  lt::settings_pack settings;
  // 调整发送/接收缓冲区
  settings.set_int(lt::settings_pack::send_buffer_watermark, 1 * 1024 * 1024);
  settings.set_int(lt::settings_pack::send_buffer_low_watermark, 16 * 1024);
  settings.set_int(lt::settings_pack::send_buffer_watermark_factor, 10);

  // 限制连接和种子数量
  settings.set_int(lt::settings_pack::connections_limit, 50);
  settings.set_int(lt::settings_pack::active_downloads, 3);
  settings.set_int(lt::settings_pack::active_seeds, 2);

  // 禁用 uTP
  settings.set_bool(lt::settings_pack::enable_outgoing_utp, false);
  settings.set_bool(lt::settings_pack::enable_incoming_utp, false);

  // 禁用不必要功能
  settings.set_bool(lt::settings_pack::enable_lsd, false);
  settings.set_bool(lt::settings_pack::enable_dht, false);
  settings.set_bool(lt::settings_pack::enable_upnp, false);
  settings.set_bool(lt::settings_pack::enable_natpmp, false);
  session.apply_settings(settings);
}

void torrent_downloader::check_torrent_helper(lt::session &session,
                                              lt::torrent_status &st, indicators::ProgressBar &progress_bar)
{
  bool torrent_finished = false; // 状态变量，标记是否完成
  while (!torrent_finished) {
    session.post_torrent_updates();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // std::vector<lt::alert *> alerts;
    session.pop_alerts(&alerts);

    for (auto alert : alerts) {
      switch (alert->type()) {
      case lt::add_torrent_alert::alert_type: {
        auto add_alert = lt::alert_cast<lt::add_torrent_alert>(alert);
        if (add_alert) {
          std::cout << "Torrent added: " << add_alert->torrent_name() << "\n";
        }
        break;
      }

        // case lt::torrent_finished_alert::alert_type:
        // {
        //   auto finished_alert =
        //   lt::alert_cast<lt::torrent_finished_alert>(alert); if
        //   (finished_alert)
        //   {
        //     std::cout << "Torrent finished\n";
        //     torrent_finished = true; // 标记完成
        //     break;
        //   }
        // }

      case lt::torrent_checked_alert::alert_type:
      {
        auto checked_alert = lt::alert_cast<lt::torrent_checked_alert>(alert);
        if (checked_alert)
        {
          std::cout << "Torrent checked\n";
        }
        break;
      }

      case lt::state_update_alert::alert_type: {
        auto update_alert = lt::alert_cast<lt::state_update_alert>(alert);
        if (update_alert && !update_alert->status.empty()) {
          st = update_alert->status.at(0);
          progress_bar.set_progress(st.progress * 100);
          double download_rate = st.download_rate / 1024.0;
          std::string speed_str =
              (download_rate > 1024.0)
                  ? std::to_string(download_rate / 1024.0) + "MB/s"
                  : std::to_string(download_rate) + "KB/s";
          progress_bar.set_option(indicators::option::PostfixText{
              "Downloading... " + std::to_string(int(st.progress * 100)) +
              "% | Speed: " + speed_str});
          if (st.is_finished) {
            progress_bar.set_option(
                indicators::option::ForegroundColor{indicators::Color::cyan});
            progress_bar.set_option(
                indicators::option::PostfixText{"Download complete!"});
            progress_bar.mark_as_completed();
            std::cout << "\nDownload complete!\n";
            torrent_finished = true;
          }
        }
        break;
      }

      default:
        break;
      }
    }
  }
}

void torrent_downloader::check_torrent() {
  for (auto &task : tasks) {
    std::cout << "torrent file: " << task.params.ti->name() << std::endl;
    check_torrent_helper(task.session, task.status, *task.progress_bar);
  }
}

void torrent_downloader::async_bitorrent_download() {
  // session.apply_settings(settings);
  // set_session(session);
  for (auto &task : tasks) {
    task.session.async_add_torrent(task.params);
    task.session.post_torrent_updates();
  }
  return;
}
