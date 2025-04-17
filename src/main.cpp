#include "downloader.h"
#include "configer.h"
#include "json/json.h"
#include "../utils/Jsonhelper.hpp"
#include <fstream>

int main(int argc, char *argv[]) {
    JsonHelper helper;
    helper.config_init("config.json");
    std::string save_path = helper.read<std::string>("save_path");
    std::vector<std::string> urls = helper.read<std::vector<std::string> >("download_urls");

    configer config_init(argc, argv);

    if (!config_init.save_path.empty()) {
        save_path = config_init.save_path;
    }
    std::cout << "Save file: " << save_path << "\n";
    std::cout << "Download file: " << config_init.file_paths.size() << "\n";

    torrent_downloader td(config_init.file_paths, save_path); // 通过本地torrent文件下载
    // torrent_downloader td(urls, save_path); // 通过url下载
    td.async_bitorrent_download();
    td.wait();
}
