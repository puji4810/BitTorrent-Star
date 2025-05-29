#include "torrent_engine.h"

namespace puji
{

    // 定义静态成员变量
    std::vector<std::string> TaskManager::trackers{};

    // 实现Task结构体的方法
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

    // TaskManager 构造函数和析构函数
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
        // 使用存储的设置值
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

        // 记录使用的设置
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

        // 使用单个 session 而不是每个任务的 session
        lt::torrent_status &st = task.status;
        std::size_t bars_index = task.bar_index;
        std::vector<lt::alert *> &alerts = task.alerts;

        // 检查 torrent_handle 是否有效
        if (!task.handle.is_valid())
        {
            spdlog::warn("Invalid torrent handle for task");
            return;
        }

        // 获取最新状态
        st = task.handle.status();

        session_->post_torrent_updates();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        session_->pop_alerts(&alerts);
        for (auto alert : alerts)
        {
            switch (alert->type())
            {
            // 处理 torrent 错误
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
                    // 遍历所有状态更新，找到匹配的任务
                    for (const auto &status : update_alert->status)
                    {
                        // 检查这个状态是否对应当前任务
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
                            break; // 找到匹配的状态，跳出内层循环
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

    void TaskManager::check_torrent_polling()
    {
        try
        {
            while (!tasks_.empty())
            {
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
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (i < tasks_.size() && !tasks_[i].is_finished.load())
                    {
                        check_torrent_status(tasks_[i]);
                    }
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
