#include "downloader.h"
#include "configer.h"
#include "json/json.h"
#include "../utils/Jsonhelper.hpp"
#include "../utils/Loghelper.hpp"
#include <fstream>

int main(int argc, char *argv[])
{
    Loginit();

    // Json config
    JsonHelper helper;
    helper.config_init("config.json");
    std::string save_path = helper.read<std::string>("save_path");
    std::vector<std::string> urls = helper.read<std::vector<std::string>>("download_urls");
    puji::TaskManager::trackers = helper.read<std::vector<std::string>>("trackers");

    // cmd config
    configer config_init(argc, argv);
    if (!config_init.save_path.empty())
    {
        save_path = config_init.save_path;
    }
    std::cout << "Save file: " << save_path << "\n";

    // manget_uri + file_path
    std::vector<std::string> src;
    src.insert(src.end(), config_init.file_paths.begin(), config_init.file_paths.end());
    src.insert(src.end(), urls.begin(), urls.end());

    const size_t batch_size = 10; // 每 10 个资源一组
    std::vector<std::thread> download_threads;

    for (size_t i = 0; i < src.size(); i += batch_size)
    {
        auto chunk_view = src | std::views::drop(i) | std::views::take(batch_size);
        std::vector<std::string> batch(chunk_view.begin(), chunk_view.end());
        for (const auto &s : batch)
        {
            spdlog::info("Download file: {}", s);
        }
        download_threads.emplace_back([batch, &save_path]()
                                      {
            puji::TorrentDownloader td(batch, save_path);
            td.async_bitorrent_download();
            td.wait(); });
    }

    for (auto &t : download_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}
