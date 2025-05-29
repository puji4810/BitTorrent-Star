#ifndef BITTORRENT_DOWNLOAD_MANAGER_H
#define BITTORRENT_DOWNLOAD_MANAGER_H

#include "torrent_engine.h"
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
		TaskManager::SessionSettings load_session_settings(const JsonHelper &helper);

		std::string save_path_;
		std::vector<std::string> download_urls_;
		std::unique_ptr<TaskManager> task_manager_;
	};

} // namespace puji

#endif // BITTORRENT_DOWNLOAD_MANAGER_H