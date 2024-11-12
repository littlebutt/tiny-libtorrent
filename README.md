# Tiny-Libtorrent: The Simplest Bittorrent Client/Library in C

Tiny-Libtorrent is a project for studying [Bittorrent Protocol](https://wiki.theory.org/BitTorrentSpecification).
It implements the most part of Bittorrent Protocol in C and can be used in limited environment like embeded systems.


## Prerequisite

Generally, the project can be run in all C environments but the recommended ways are different. If you are a Linux/BSD
user, compiling with the GCC toolchain is a better choice. If you are a Windows user, you can use Visual Studio 2022 to
compile the project.


## Compiling

If you are using GCC, run `mkdir build && make` in the root directory. If you are a Windows user, double click `.sln` file to open the
project and run it in `x64` mode.


## Usage

In Linux or BSD, you can simply run the following command in shell then it will download the content in `torrent_filename`.

```Shell
tiny-libtorrent torrent_filename
```

Otherwise in Windows, you can run this command.

```Shell
tiny-libtorrent.exe torrent_filename
```

The torrent_filename must be a `.torrent` file.


## Limitation

The project does not support the following features:

1. Multi-files torrent
2. Magnet link
3. Uploading (Seeding)
4. Resume from breakpoint
5. Coroutine (In progress)
6. NAT traversal
7. SSL/TLS protocol


## References:

1. [The Bittorrent Protocol implementation in GO](https://blog.jse.li/posts/torrent/)
2. [The Bittorrent Protocol Specification](https://wiki.theory.org/BitTorrentSpecification)
3. [The Libtorrent Project](https://www.libtorrent.org/)
4. ChatGPT (the most gratefull)
