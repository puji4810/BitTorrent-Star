#ifndef BITTORRENT_DOWNLOAD_MANAGER_H
#define BITTORRENT_DOWNLOAD_MANAGER_H

#include "torrent_engine.h"
#include "../utils/Jsonhelper.hpp"
#include "../utils/Loghelper.hpp"
#include "cli_options.h"
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
		DownloadManager(const std::string &config_path, const configer cli_config = configer(0, nullptr));
		void start_downloads(std::atomic<bool> &shutdown_flag);

	private:
		void load_config(const std::string &config_path);
		void add_all_tasks();
		TaskManager::SessionSettings load_session_settings(const JsonHelper &helper);

		std::string save_path_;
		std::vector<std::string> resources_;
		std::unique_ptr<TaskManager> task_manager_;
	};

} // namespace puji

#endif // BITTORRENT_DOWNLOAD_MANAGER_H