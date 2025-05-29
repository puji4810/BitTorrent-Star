#include "torrent_download_manager.h"
#include "spdlog/spdlog.h"

namespace puji
{

	DownloadManager::DownloadManager(const std::string &config_path)
	{
		load_config(config_path);
	}

	void DownloadManager::start_downloads()
	{
		spdlog::info("Starting downloads for {} URLs", download_urls_.size());

		add_all_tasks();
		task_manager_->async_bitorrent_download();
		task_manager_->check_torrent_polling();

		spdlog::info("All downloads completed");
	}

	TaskManager::SessionSettings DownloadManager::load_session_settings(const JsonHelper &helper)
	{
		TaskManager::SessionSettings settings;

		if (helper.is_json_member(helper.get_config(), "session_settings"))
		{
			// 从配置文件中读取设置
			settings.send_buffer_watermark = helper.read<int>("session_settings.send_buffer_watermark");
			settings.send_buffer_low_watermark = helper.read<int>("session_settings.send_buffer_low_watermark");
			settings.send_buffer_watermark_factor = helper.read<int>("session_settings.send_buffer_watermark_factor");
			settings.connections_limit = helper.read<int>("session_settings.connections_limit");
			settings.active_downloads = helper.read<int>("session_settings.active_downloads");
			settings.active_seeds = helper.read<int>("session_settings.active_seeds");
			settings.enable_dht = helper.read<bool>("session_settings.enable_dht");
			settings.dht_upload_rate_limit = helper.read<int>("session_settings.dht_upload_rate_limit");

			Json::Value alert_mask_value = helper.get_nested_value("session_settings.alert_mask");
			spdlog::info("Raw alert_mask value: {}", alert_mask_value.asString());
			spdlog::info("alert_mask as asUInt(): {}", alert_mask_value.asUInt());

			settings.alert_mask = helper.read<std::uint32_t>("session_settings.alert_mask");
			spdlog::info("Loaded alert_mask as uint32_t: {}", settings.alert_mask);

			spdlog::info("Loaded session settings from config.json");
		}
		else
		{
			spdlog::info("Using default session settings");
		}

		return settings;
	}

	void DownloadManager::load_config(const std::string &config_path)
	{
		JsonHelper helper;
		helper.config_init(config_path);

		save_path_ = helper.read<std::string>("save_path");
		download_urls_ = helper.read<std::vector<std::string>>("download_urls");
		TaskManager::trackers = helper.read<std::vector<std::string>>("trackers");

		// 加载会话设置并初始化 TaskManager
		TaskManager::SessionSettings settings = load_session_settings(helper);
		task_manager_ = std::make_unique<TaskManager>(settings);

		spdlog::info("Configuration loaded successfully");
	}

	void DownloadManager::add_all_tasks()
	{
		spdlog::info("Adding {} tasks to TaskManager", download_urls_.size());

		for (const auto &url : download_urls_)
		{
			task_manager_->add_task(url, save_path_);
			spdlog::info("Added task: {}", url);
		}
	}

} // namespace puji