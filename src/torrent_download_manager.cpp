#include "torrent_download_manager.h"
#include "spdlog/spdlog.h"

namespace puji
{

	DownloadManager::DownloadManager(const std::string &config_path, const configer cli_config)
		: save_path_(""), resources_(), task_manager_(nullptr)
	{
		load_config(config_path);

		if (!cli_config.save_path.empty())
		{
			save_path_ = cli_config.save_path;
			spdlog::info("Using save path from command line: {}", save_path_);
		}

		if (!cli_config.resource.empty())
		{
			resources_ = cli_config.resource;
			spdlog::info("Using download source from command line");
		}
		else
		{
			spdlog::info("No download source provided in command line, will load from config file");
		}
	}

	void DownloadManager::start_downloads(std::atomic<bool> &shutdown_flag)
	{
		spdlog::info("Starting downloads for {} URLs", resources_.size());

		add_all_tasks();
		task_manager_->async_bitorrent_download();

		task_manager_->check_torrent_polling(shutdown_flag);

		if (shutdown_flag && shutdown_flag.load())
		{
			spdlog::info("正在保存断点续传数据...");
			task_manager_->save_all_resume_data();
		}

		spdlog::info("All downloads completed or interrupted");
	}

	TaskManager::SessionSettings DownloadManager::load_session_settings(const JsonHelper &helper)
	{
		TaskManager::SessionSettings settings;

		if (helper.is_json_member(helper.get_config(), "session_settings"))
		{
			settings.send_buffer_watermark = helper.read<int>("session_settings.send_buffer_watermark");
			settings.send_buffer_low_watermark = helper.read<int>("session_settings.send_buffer_low_watermark");
			settings.send_buffer_watermark_factor = helper.read<int>("session_settings.send_buffer_watermark_factor");
			settings.connections_limit = helper.read<int>("session_settings.connections_limit");
			settings.active_downloads = helper.read<int>("session_settings.active_downloads");
			settings.active_seeds = helper.read<int>("session_settings.active_seeds");
			settings.enable_dht = helper.read<bool>("session_settings.enable_dht");
			settings.dht_upload_rate_limit = helper.read<int>("session_settings.dht_upload_rate_limit");
			settings.alert_mask = helper.read<std::uint32_t>("session_settings.alert_mask");

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
		resources_ = helper.read<std::vector<std::string>>("resources");
		TaskManager::trackers = helper.read<std::vector<std::string>>("trackers");

		TaskManager::SessionSettings settings = load_session_settings(helper);
		task_manager_ = std::make_unique<TaskManager>(settings);

		spdlog::info("Configuration loaded successfully");
	}

	void DownloadManager::add_all_tasks()
	{
		spdlog::info("Adding {} tasks to TaskManager", resources_.size());

		for (const auto &url : resources_)
		{
			task_manager_->add_task(url, save_path_);
			spdlog::info("Added task: {}", url);
		}
	}

} // namespace puji