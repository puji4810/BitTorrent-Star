#include "torrent_engine.h"

namespace puji
{
    std::vector<std::string> TaskManager::trackers{};

    TaskManager::Task::Task() = default;

    TaskManager::Task::Task(Task &&other) noexcept
        : handle(std::move(other.handle)), params(std::move(other.params)),
          status(std::move(other.status)), alerts(std::move(other.alerts)),
          bar_index(other.bar_index), is_finished(other.is_finished.load())
    {
    }

    TaskManager::Task &TaskManager::Task::operator=(Task &&other) noexcept
    {
        if (this != &other)
        {
            handle = std::move(other.handle);
            params = std::move(other.params);
            status = std::move(other.status);
            alerts = std::move(other.alerts);
            bar_index = other.bar_index;
            is_finished.store(other.is_finished.load());
        }
        return *this;
    }

    TaskManager::TaskManager()
    {
        session_ = std::make_unique<libtorrent::session>();
        configure_session();
        spdlog::info("TaskManager initialized with default settings");
    }

    TaskManager::TaskManager(const SessionSettings &settings) : settings_(settings)
    {
        session_ = std::make_unique<libtorrent::session>();
        configure_session();
        spdlog::info("TaskManager initialized with custom settings");
    }

    TaskManager::~TaskManager()
    {
        spdlog::info("TaskManager destructor called");
    }

    void TaskManager::configure_session()
    {
        lt::settings_pack settings;
        settings.set_int(lt::settings_pack::send_buffer_watermark, settings_.send_buffer_watermark);
        settings.set_int(lt::settings_pack::send_buffer_low_watermark, settings_.send_buffer_low_watermark);
        settings.set_int(lt::settings_pack::send_buffer_watermark_factor, settings_.send_buffer_watermark_factor);

        // 限制连接和种子数量
        settings.set_int(lt::settings_pack::connections_limit, settings_.connections_limit);
        settings.set_int(lt::settings_pack::active_downloads, settings_.active_downloads);
        settings.set_int(lt::settings_pack::active_seeds, settings_.active_seeds);

        settings.set_bool(lt::settings_pack::enable_dht, settings_.enable_dht);
        settings.set_int(lt::settings_pack::dht_upload_rate_limit, settings_.dht_upload_rate_limit);

        settings.set_int(lt::settings_pack::alert_mask, settings_.alert_mask);

        session_->apply_settings(settings);

        spdlog::info("Session configured with settings:");
        spdlog::info("  - connections_limit: {}", settings_.connections_limit);
        spdlog::info("  - active_downloads: {}", settings_.active_downloads);
        spdlog::info("  - active_seeds: {}", settings_.active_seeds);
    }

    void TaskManager::check_torrent_status(Task &task)
    {
        if (task.is_finished.load())
        {
            return;
        }

        lt::torrent_status &st = task.status;
        std::size_t bars_index = task.bar_index;
        std::vector<lt::alert *> &alerts = task.alerts;

        if (!task.handle.is_valid())
        {
            spdlog::warn("Invalid torrent handle for task");
            return;
        }

        st = task.handle.status();

        session_->post_torrent_updates();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        session_->pop_alerts(&alerts);
        for (auto alert : alerts)
        {
            switch (alert->type())
            {
            case lt::torrent_error_alert::alert_type:
            {
                auto error_alert = lt::alert_cast<lt::torrent_error_alert>(alert);
                if (error_alert)
                {
                    spdlog::error("Torrent error: {}", error_alert->error.message());
                }
                break;
            }

            // 处理元数据解析失败
            case lt::metadata_failed_alert::alert_type:
            {
                auto metadata_alert = lt::alert_cast<lt::metadata_failed_alert>(alert);
                if (metadata_alert)
                {
                    spdlog::error("Failed to retrieve metadata for torrent: {}. Error: {}",
                                  metadata_alert->handle.info_hashes().get_best().to_string(),
                                  metadata_alert->error.message());
                }
                break;
            }

            // 处理 tracker 错误
            case lt::tracker_error_alert::alert_type:
            {
                auto tracker_alert = lt::alert_cast<lt::tracker_error_alert>(alert);
                if (tracker_alert)
                {
                    spdlog::error("Tracker error: {}", tracker_alert->error.message());
                }
                break;
            }

            // 处理文件错误
            case lt::file_error_alert::alert_type:
            {
                auto file_alert = lt::alert_cast<lt::file_error_alert>(alert);
                if (file_alert)
                {
                    spdlog::error("File error: {}", file_alert->error.message());
                }
                break;
            }

            // 其他已有的警报处理逻辑
            case lt::torrent_checked_alert::alert_type:
            {
                auto checked_alert = lt::alert_cast<lt::torrent_checked_alert>(alert);
                if (checked_alert)
                {
                    spdlog::info("Torrent checked: {}",
                                 checked_alert->handle.info_hashes().get_best().to_string());
                }
                break;
            }

            case lt::state_update_alert::alert_type:
            {
                auto update_alert = lt::alert_cast<lt::state_update_alert>(alert);
                if (update_alert && !update_alert->status.empty())
                {
                    for (const auto &status : update_alert->status)
                    {
                        if (task.handle.is_valid() && task.handle.info_hashes() == status.handle.info_hashes())
                        {
                            st = status;
                            ProgressManager::getInstance().get_progress_bar(bars_index).set_progress(st.progress * 100);
                            double download_rate = st.download_rate / 1024.0;
                            std::string speed_str =
                                (download_rate > 1024.0)
                                    ? std::to_string(download_rate / 1024.0) + "MB/s"
                                    : std::to_string(download_rate) + "KB/s";
                            ProgressManager::getInstance().get_progress_bar(bars_index).set_option(indicators::option::PostfixText{"Downloading... " + std::to_string(static_cast<int>(st.progress * 100)) + "% | Speed: " + speed_str + " | [" + st.name.substr(0, 40) + "]"});
                            if (st.is_finished)
                            {
                                ProgressManager::getInstance().get_progress_bar(bars_index).set_option(indicators::option::ForegroundColor{indicators::Color::cyan});
                                ProgressManager::getInstance().get_progress_bar(bars_index).set_option(indicators::option::PostfixText{"Download complete!"});
                                ProgressManager::getInstance().get_progress_bar(bars_index).mark_as_completed();
                                task.is_finished.store(true);
                                spdlog::info("Task completed: {}", st.name);
                                return;
                            }
                            break;
                        }
                    }
                }
                break;
            }

            default:
                break;
            }
        }
    }

    void TaskManager::check_torrent_polling(std::atomic<bool> &shutdown_flag)
    {
        try
        {
            while (!tasks_.empty())
            {
                if (shutdown_flag && shutdown_flag.load())
                {
                    spdlog::info("检测到退出信号，中断轮询");
                    break;
                }

                std::vector<std::size_t> task_indices;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    for (std::size_t i = 0; i < tasks_.size(); ++i)
                    {
                        if (!tasks_[i].is_finished.load())
                        {
                            task_indices.push_back(i);
                        }
                    }
                }

                for (std::size_t i : task_indices)
                {
                    if (shutdown_flag && shutdown_flag.load())
                    {
                        spdlog::info("检测到退出信号，中断任务检查");
                        break;
                    }

                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        if (i < tasks_.size() && !tasks_[i].is_finished.load())
                        {
                            check_torrent_status(tasks_[i]);
                        }
                    }
                }

                if (shutdown_flag && shutdown_flag.load())
                {
                    break;
                }

                remove_finished_tasks();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            spdlog::info("TaskManager polling completed");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error in check_torrent_polling: {}", e.what());
        }
    }

    void TaskManager::async_bitorrent_download()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        spdlog::info("Starting async download for {} tasks", tasks_.size());

        for (auto &task : tasks_)
        {
            if (!task.is_finished.load())
            {
                task.handle = session_->add_torrent(task.params);
                if (task.handle.is_valid())
                {
                    spdlog::info("Added torrent: {}", task.params.name);
                }
                else
                {
                    spdlog::error("Failed to add torrent: {}", task.params.name);
                }
            }
        }

        session_->post_torrent_updates();
    }

    void TaskManager::add_task(const std::string &src, const std::string &save_path)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        spdlog::info("Adding new task: {}", src);

        libtorrent::add_torrent_params params;
        Task tk;

        if (is_magnet_uri(src))
        {
            params = libtorrent::parse_magnet_uri(src);
            spdlog::info("Parsed magnet URI");
        }
        else
        {
            try
            {
                params.ti = std::make_shared<libtorrent::torrent_info>(src);
                spdlog::info("Loaded torrent file");
            }
            catch (const std::exception &e)
            {
                spdlog::error("Failed to load torrent file: {}", e.what());
                return;
            }
        }

        params.trackers = trackers;
        params.save_path = save_path;
        tk.params = params;

        // 添加进度条并获取索引
        tk.bar_index = ProgressManager::getInstance().add_progress_bar();

        tasks_.emplace_back(std::move(tk));
        spdlog::info("Task added successfully");
    }

    void TaskManager::remove_finished_tasks()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::erase_if(tasks_, [](const auto &task)
                      { return task.is_finished.load(); });
    }

    void TaskManager::save_all_resume_data()
    {
        spdlog::info("正在保存所有任务的断点续传数据...");

        std::atomic<int> outstanding_resume_data{0};

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto &task : tasks_)
            {
                if (task.handle.is_valid())
                {
                    try
                    {
                        task.handle.save_resume_data(lt::torrent_handle::only_if_modified);
                        ++outstanding_resume_data;
                        spdlog::info("请求保存任务断点续传数据: {}", task.params.name);
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("保存断点续传数据失败: {}", e.what());
                    }
                }
            }
        }

        while (outstanding_resume_data > 0)
        {
            std::vector<lt::alert *> alerts;
            session_->wait_for_alert(std::chrono::seconds(30));
            session_->pop_alerts(&alerts);

            for (lt::alert *a : alerts)
            {
                if (auto *rd_alert = lt::alert_cast<lt::save_resume_data_alert>(a))
                {
                    auto &params = rd_alert->params;
                    std::string filename;

                    if (!params.name.empty())
                    {
                        filename = params.name + ".fastresume";
                    }
                    else if (params.ti)
                    {
                        filename = params.ti->name() + ".fastresume";
                    }
                    else
                    {
                        filename = params.info_hashes.get_best().to_string() + ".fastresume";
                    }

                    std::string full_path = params.save_path + "/" + filename;

                    try
                    {
                        std::vector<char> buf = lt::write_resume_data_buf(params);
                        std::ofstream out(full_path, std::ios_base::binary);
                        out.write(buf.data(), buf.size());
                        spdlog::info("断点续传数据已保存至: {}", full_path);
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("写入断点续传文件失败: {}", e.what());
                    }

                    --outstanding_resume_data;
                }
                else if (lt::alert_cast<lt::save_resume_data_failed_alert>(a))
                {
                    spdlog::warn("保存断点续传数据失败");
                    --outstanding_resume_data;
                }
            }
        }

        spdlog::info("所有断点续传数据已保存");
    }

    bool TaskManager::empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return tasks_.empty();
    }

    std::vector<TaskManager::Task> &TaskManager::get_tasks()
    {
        return tasks_;
    }

} // namespace puji
