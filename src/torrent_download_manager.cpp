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

		// 添加所有任务到单个 TaskManager
		add_all_tasks();

		// 启动下载
		task_manager_.async_bitorrent_download();

		// 等待所有下载完成
		task_manager_.check_torrent_polling();

		spdlog::info("All downloads completed");
	}

	void DownloadManager::load_config(const std::string &config_path)
	{
		JsonHelper helper;
		helper.config_init(config_path);

		save_path_ = helper.read<std::string>("save_path");
		download_urls_ = helper.read<std::vector<std::string>>("download_urls");
		TaskManager::trackers = helper.read<std::vector<std::string>>("trackers");
	}

	void DownloadManager::add_all_tasks()
	{
		spdlog::info("Adding {} tasks to TaskManager", download_urls_.size());

		for (const auto &url : download_urls_)
		{
			task_manager_.add_task(url, save_path_);
			spdlog::info("Added task: {}", url);
		}
	}

} // namespace puji