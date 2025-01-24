#include "downloader.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <torrent-file>\n";
    return 1;
  } else {
    torrent_downloader td{argv[1], "./download"};
    td.async_bitorrent_download();
    td.wait();
    // bitorrent_download(argc, argv);
  }
}
