#include "downloader.h"

std::vector<std::string> torrent_downloader::trackers{};

std::size_t torrent_downloader::bar_index = 0;

indicators::DynamicProgress<indicators::ProgressBar> torrent_downloader::bars{};

void torrent_downloader::set_session(lt::session &session) {
    lt::settings_pack settings;
    // 调整发送/接收缓冲区
    settings.set_int(lt::settings_pack::send_buffer_watermark, 1 * 1024 * 1024);
    settings.set_int(lt::settings_pack::send_buffer_low_watermark, 16 * 1024);
    settings.set_int(lt::settings_pack::send_buffer_watermark_factor, 10);

    // 限制连接和种子数量
    settings.set_int(lt::settings_pack::connections_limit, 50);
    settings.set_int(lt::settings_pack::active_downloads, 10);
    settings.set_int(lt::settings_pack::active_seeds, 10);

    settings.set_bool(lt::settings_pack::enable_dht, true); // 确保DHT功能启用
    settings.set_int(lt::settings_pack::dht_upload_rate_limit, 0); // 不限制DHT上传速率

    settings.set_int(lt::settings_pack::alert_mask, lt::alert::all_categories);

    session.apply_settings(settings);
}

void torrent_downloader::check_torrent_status(Task &tk) {
    lt::session &session = *tk.session;
    lt::torrent_status &st = tk.status;
    std::size_t bars_index = tk.bar_index;
    std::vector<lt::alert *> &alerts = tk.alerts;

    session.post_torrent_updates();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    session.pop_alerts(&alerts);
    for (auto alert: alerts) {
        switch (alert->type()) {
            // 处理 torrent 错误
            case lt::torrent_error_alert::alert_type: {
                auto error_alert = lt::alert_cast<lt::torrent_error_alert>(alert);
                if (error_alert) {
                    spdlog::error("Torrent error: {}", error_alert->error.message());
                }
                break;
            }

            // 处理元数据解析失败
            case lt::metadata_failed_alert::alert_type: {
                auto metadata_alert = lt::alert_cast<lt::metadata_failed_alert>(alert);
                if (metadata_alert) {
                    spdlog::error("Failed to retrieve metadata for torrent: {}. Error: {}",
                                  metadata_alert->handle.info_hashes().get_best().to_string(),
                                  metadata_alert->error.message());
                }
                break;
            }

            // 处理 tracker 错误
            case lt::tracker_error_alert::alert_type: {
                auto tracker_alert = lt::alert_cast<lt::tracker_error_alert>(alert);
                if (tracker_alert) {
                    spdlog::error("Tracker error: {}", tracker_alert->error.message());
                }
                break;
            }

            // 处理文件错误
            case lt::file_error_alert::alert_type: {
                auto file_alert = lt::alert_cast<lt::file_error_alert>(alert);
                if (file_alert) {
                    spdlog::error("File error: {}",
                                  file_alert->error.message());
                }
                break;
            }

            // 其他已有的警报处理逻辑
            case lt::torrent_checked_alert::alert_type: {
                auto checked_alert = lt::alert_cast<lt::torrent_checked_alert>(alert);
                if (checked_alert) {
                    spdlog::info("Torrent checked: {}",
                                 checked_alert->handle.info_hashes().get_best().to_string());
                }
                break;
            }

            case lt::state_update_alert::alert_type: {
                auto update_alert = lt::alert_cast<lt::state_update_alert>(alert);
                if (update_alert && !update_alert->status.empty()) {
                    st = update_alert->status.at(0);
                    if ( tk.params.name != st.name) {
                        break;
                    }
                    bars[bars_index].set_progress(st.progress * 100);
                    double download_rate = st.download_rate / 1024.0;
                    std::string speed_str =
                            (download_rate > 1024.0)
                                ? std::to_string(download_rate / 1024.0) + "MB/s"
                                : std::to_string(download_rate) + "KB/s";
                    bars[bars_index].set_option(indicators::option::PostfixText{
                        "Downloading... " + std::to_string(static_cast<int>(st.progress * 100)) +
                        "% | Speed: " + speed_str + " | [" + st.name.substr(0, 40) + "]"
                    });
                    if (st.is_finished) {
                        bars[bars_index].set_option(
                            indicators::option::ForegroundColor{indicators::Color::cyan});
                        bars[bars_index].set_option(
                            indicators::option::PostfixText{"Download complete!"});
                        bars[bars_index].mark_as_completed();
                        tk.is_finished = true;
                        return;
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

void torrent_downloader::check_torrent_polling() {
    while (!tasks.empty()) {
        for (size_t i = 0; i < tasks.size(); ++i) {
            check_torrent_status(tasks[i]);
        }

        std::erase_if(tasks, [](const auto &task) {
            return task.is_finished;
        });
    }
}


void torrent_downloader::async_bitorrent_download() {
    for (auto &task: tasks) {
        // set_session(task.session);
        task.session->async_add_torrent(task.params);
        task.session->post_torrent_updates();
    }
}
