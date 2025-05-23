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
            ("help", "produce help message")
            ("version,v","print version string")
            ("save,s", po::value<std::string>()/*->default_value("./download/")*/,"saved path")
            ("download,d", po::value<std::vector<std::string> >(),"download file")
            ("add a", po::value<std::vector<std::string> >(), "add download url");

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
        if (vm.count("save")) {
            save_path = vm["save"].as<std::string>();
        }
        if (vm.count("download")) {
            file_paths = vm["download"].as<std::vector<std::string> >();
        }
        if (vm.count("add")) {
            download_urls = vm["add"].as<std::vector<std::string> >();
        }
    }
};
