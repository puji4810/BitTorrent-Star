#include "download_manager.h"
#include "spdlog/spdlog.h"

namespace puji
{

	DownloadManager::DownloadManager(const std::string &config_path)
	{
		load_config(config_path);
	}

	void DownloadManager::start_downloads()
	{
		const size_t batch_size = 10;
		auto batches = create_batches(batch_size);

		std::vector<std::thread> download_threads;
		download_threads.reserve(batches.size());

		for (const auto &batch : batches)
		{
			download_threads.emplace_back([this, batch]()
										  { process_batch(batch); });
		}

		wait_for_completion(download_threads);
	}

	void DownloadManager::load_config(const std::string &config_path)
	{
		JsonHelper helper;
		helper.config_init(config_path);

		save_path_ = helper.read<std::string>("save_path");
		download_urls_ = helper.read<std::vector<std::string>>("download_urls");
		TaskManager::trackers = helper.read<std::vector<std::string>>("trackers");
	}

	std::vector<std::vector<std::string>> DownloadManager::create_batches(size_t batch_size)
	{
		std::vector<std::vector<std::string>> batches;
		for (size_t i = 0; i < download_urls_.size(); i += batch_size)
		{
			auto chunk_view = download_urls_ | std::views::drop(i) | std::views::take(batch_size);
			batches.emplace_back(chunk_view.begin(), chunk_view.end());
		}
		return batches;
	}

	void DownloadManager::process_batch(const std::vector<std::string> &batch)
	{
		for (const auto &url : batch)
		{
			spdlog::info("Download file: {}", url);
		}

		TorrentDownloader td(batch, save_path_);
		td.async_bitorrent_download();
		td.wait();
	}

	void DownloadManager::wait_for_completion(std::vector<std::thread> &threads)
	{
		std::for_each(threads.begin(), threads.end(), [](auto &t)
					  {
        if (t.joinable()) {
            t.join();
        } });
	}

} // namespace puji