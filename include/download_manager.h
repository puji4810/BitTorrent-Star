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
		std::vector<std::vector<std::string>> create_batches(size_t batch_size);
		void process_batch(const std::vector<std::string> &batch);
		void wait_for_completion(std::vector<std::thread> &threads);

		std::string save_path_;
		std::vector<std::string> download_urls_;
	};

} // namespace puji

#endif // BITTORRENT_DOWNLOAD_MANAGER_H