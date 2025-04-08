#include "downloader.h"
#include "initer.h"
#include "json/json.h"
#include <fstream>

int main(int argc, char *argv[]) {
  Json::Value config;
  std::ifstream config_file("config.json");
  if (!config_file.is_open()) {
    std::cerr << "Could not open config.json file." << std::endl;
    return 1;
  }
  config_file >> config;
  config_file.close();
  std::string save_path = config["save_path"].asString();

  initer initer(argc, argv);
  if (!initer.save_path.empty()) {
    save_path = initer.save_path;
  }
  std::cout << "Save file: " << save_path << "\n";
  torrent_downloader td(initer.file_paths, save_path);
  td.async_bitorrent_download();
  td.wait();
}
