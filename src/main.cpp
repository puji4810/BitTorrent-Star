#include "torrent_download_manager.h"
#include "json/json.h"
#include "../utils/Jsonhelper.hpp"
#include "../utils/Loghelper.hpp"
#include <fstream>
#include <ranges>
#include <algorithm>
#include "spdlog/spdlog.h"
#include <csignal>
#include <atomic>

std::atomic<bool> g_shutdown_flag{false};

void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        spdlog::info("接收到退出信号，准备安全终止...");
        g_shutdown_flag.store(true);
    }
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        Loginit();
        configer config_init(argc, argv);
        puji::DownloadManager manager("config.json", config_init);
        manager.start_downloads(g_shutdown_flag);
        return 0;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
}
