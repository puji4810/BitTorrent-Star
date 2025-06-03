//
// Created by 刘智禹 on 25-4-21.
//

#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
#include <filesystem>

inline void Loginit()
{
    try
    {
        std::filesystem::create_directories("logs");
        auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/logfile.log");
        file_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        spdlog::set_default_logger(file_logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_level(spdlog::level::debug);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

#endif // LOGHELPER_H
