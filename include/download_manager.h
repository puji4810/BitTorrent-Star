#ifndef BITTORRENT_DOWNLOAD_MANAGER_H
#define BITTORRENT_DOWNLOAD_MANAGER_H

#include "downloader.h"
#include "../utils/Jsonhelper.hpp"
#include "../utils/Loghelper.hpp"
#include <string>
#include <vector>
#include <thread>
#include <ranges>
#include <algorithm>

namespace puji
{

	class DownloadManager
	{
	public:
		explicit DownloadManager(const std::string &config_path);
		void start_downloads();

	private:
		void load_config(const std::string &config_path);
		void add_all_tasks();

		std::string save_path_;
		std::vector<std::string> download_urls_;
		TaskManager task_manager_; // 单个 TaskManager 实例
	};

} // namespace puji

#endif // BITTORRENT_DOWNLOAD_MANAGER_H