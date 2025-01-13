
#include <chrono>
#include <iostream>
#include <thread>

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_handle.hpp>

#include <indicators/progress_bar.hpp>

inline int bitorrent_download(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <torrent-file>\n";
    return 1;
  }

  // 创建 libtorrent 会话
  lt::session session;

  // 加载 .torrent 文件
  auto ti = std::make_shared<lt::torrent_info>(argv[1]);

  // 显示种子文件信息
  std::cout << "Torrent name: " << ti->name() << "\n";
  std::cout << "Number of files: " << ti->num_files() << "\n";

  // 设置种子参数
  lt::add_torrent_params params;
  params.ti = ti;
  params.save_path = "./downloads"; // 下载保存路径
  auto handle = session.add_torrent(params);

  // 设置进度条
  indicators::ProgressBar progress_bar{
      indicators::option::BarWidth{50},
      indicators::option::Start{"["},
      indicators::option::Fill{"="},
      indicators::option::Lead{">"},
      indicators::option::Remainder{" "},
      indicators::option::End{"]"},
      indicators::option::ForegroundColor{indicators::Color::green},
      indicators::option::ShowElapsedTime{true},
      indicators::option::ShowRemainingTime{true},
      indicators::option::PostfixText{"Initializing..."}};

  // 打印下载进度
  std::this_thread::sleep_for(std::chrono::seconds(1));

  auto status = handle.status();

  // 获取下载速率（字节/秒）
  double download_rate_kbps = status.download_rate / 1024.0;

  // 更新进度条
  progress_bar.set_progress(status.progress * 100);
  progress_bar.set_option(indicators::option::PostfixText{
      "Downloading... " + std::to_string(int(status.progress * 100)) +
      "% | Speed: " + std::to_string(download_rate_kbps) + " kB/s"});

  // 检查是否完成
  if (status.is_finished) {
    progress_bar.set_option(
        indicators::option::ForegroundColor{indicators::Color::cyan});
    progress_bar.set_option(
        indicators::option::PostfixText{"Download complete!"});
    progress_bar.mark_as_completed();
    std::cout << "\nDownload complete!\n";
    return 0;
  }
  return 0;
}
