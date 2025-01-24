#include <boost/program_options.hpp>
namespace po = boost::program_options;

struct initer {

  void options_init() {
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")(
        "torrent-file", po::value<std::string>(), "torrent file")(
        "output-dir", po::value<std::string>(), "output directory");

    po::variables_map vm;
  }
};
