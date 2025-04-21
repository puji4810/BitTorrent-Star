//
// Created by 刘智禹 on 25-4-21.
//

#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>



     void Loginit(){
        try {
            // 创建一个文件日志记录器
            auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/logfile.log");

            // 设置日志格式（可选）
            file_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

            // 设置为默认日志记录器
            spdlog::set_default_logger(file_logger);

        } catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
      }




#endif //LOGHELPER_H
