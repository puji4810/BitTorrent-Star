# BitTorrent-Star
A command-line based torrent downloader
A lightweight and efficient command-line torrent downloader written in Modern C++(maybe). This tool allows you to download torrent files directly from the terminal, making it ideal for headless servers or users who prefer a minimalistic approach.

## Features

- **Simple and Fast**: Download torrents with a single command.
- **Cross-Platform**: Works on Linux, macOS, and Windows.
- **Customizable**: Set download directory, bandwidth limits, and more.
- **Magnet Link Support**: Download torrents using magnet links.
- **Progress Tracking**: Real-time download progress and speed.

## Build

- Clone the repository:
```bash
  git clone git@github.com:puji4810/BitTorrent-Star.git
```

- Change the directory:
```bash
  cd BitTorrent-Star
  git submodule update --init --recursive
```

- Build the project:
```bash
  mkdir build
  cd build
  cmake ..
  make
```
