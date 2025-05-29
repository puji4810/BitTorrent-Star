#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>
namespace po = boost::program_options;

struct configer {
    int argc;
    char **argv;
    std::string save_path;
    std::vector<std::string> file_paths;
    std::vector<std::string> download_urls;

    configer(int argc, char **argv) : argc(argc), argv(argv) { options_init(); }

    void options_init() {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "显示帮助信息")
            ("version,v", "显示版本信息")
            ("save,s", po::value<std::string>()->default_value("./download/"), "保存路径，默认为 ./download/")
            ("download,d", po::value<std::vector<std::string>>(), "指定要下载的本地文件路径(.torrent文件)")
            ("add,a", po::value<std::vector<std::string>>(), "添加下载链接");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }
        if (vm.count("version")) {
            std::cout << "Version 1.0\n";
            exit(0);
        }
        save_path = vm["save"].as<std::string>();
        if (vm.count("download")) {
            file_paths = vm["download"].as<std::vector<std::string>>();
        }
        if (vm.count("add")) {
            download_urls = vm["add"].as<std::vector<std::string>>();
        }
    }
};
