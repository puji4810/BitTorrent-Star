#include "download.cpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <torrent-file>\n";
    return 1;
  } else {
    async_bitorrent_download(argc, argv);
    // bitorrent_download(argc, argv);
  }
}
