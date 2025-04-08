#include "downloader.h"
#include "initer.h"
#include "json/json.h"
#include <fstream>

int main(int argc, char *argv[]) {
  initer initer(argc, argv);
  torrent_downloader td(initer.file_paths, initer.save_path);
  td.async_bitorrent_download();
  td.wait();
}
