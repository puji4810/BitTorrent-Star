#include "torrent_download_manager.h"
#include "cli_options.h"
#include "json/json.h"
#include "../utils/Jsonhelper.hpp"
#include "../utils/Loghelper.hpp"
#include <fstream>
#include <ranges>
#include <algorithm>
#include "spdlog/spdlog.h"

int main(int argc, char *argv[])
{
    try
    {
        Loginit();
        
        // 处理命令行参数
        configer config_init(argc, argv);

        // 创建下载管理器
        puji::DownloadManager manager("config.json");

        // 如果命令行指定了保存路径，覆盖配置文件中的设置
        if (!config_init.save_path.empty())
        {
            spdlog::info("Using save path from command line: {}", config_init.save_path);
        }

        // 开始下载
        manager.start_downloads();

        return 0;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
}
