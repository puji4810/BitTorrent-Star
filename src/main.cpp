#include "downloader.h"
#include "configer.h"
#include "json/json.h"
#include "../utils/Jsonhelper.hpp"
#include "../utils/Loghelper.hpp"
#include <fstream>

int main(int argc, char *argv[]) {
    Loginit();

    // Json config
    JsonHelper helper;
    helper.config_init("config.json");
    std::string save_path = helper.read<std::string>("save_path");
    std::vector<std::string> urls = helper.read<std::vector<std::string> >("download_urls");
    torrent_downloader::trackers = helper.read<std::vector<std::string> >("trackers");

    // cmd config
    configer config_init(argc, argv);
    if (!config_init.save_path.empty()) {
        save_path = config_init.save_path;
    }
    std::cout << "Save file: " << save_path << "\n";

    // manget_uri + file_path
    std::vector<std::string> src;
    src.insert(src.end(), config_init.file_paths.begin(), config_init.file_paths.end());
    src.insert(src.end(), urls.begin(), urls.end());

    torrent_downloader td(src, save_path);
    td.async_bitorrent_download();
    td.wait();
}
