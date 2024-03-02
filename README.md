<div>
<img src="https://raw.githubusercontent.com/helloall1900/vhash/master/assets/logo.png"><br>
</div>

<h2 align="center"> The hash tool for duplicate video and image detection </h2>

<p>
<a href="https://github.com/helloall1900/vhash/actions?query=workflow%3AUbuntu"><img alt="Build Status" src="https://github.com/helloall1900/vhash/workflows/Ubuntu/badge.svg"></a>
<a href="https://github.com/helloall1900/vhash/actions?query=workflow%3AmacOS"><img alt="Build Status" src="https://github.com/helloall1900/vhash/workflows/macOS/badge.svg"></a>
<img alt="License" src="https://img.shields.io/github/license/helloall1900/vhash?style=plastic">
<img alt="C++" src="https://img.shields.io/badge/c%2B%2B-14-brightgreen">
</p>

--------------------------------------------------------------------------

## Introduction

**vhash** is a C++ reimplementation of [videohash](https://github.com/akamhy/videohash) for **detecting near-duplicate videos**.
It takes any input video or image file and generate a 64-bit equivalent hash value.

--------------------------------------------------------------------------

## Build vhash

### Requirements

- A C++ compiler supports **C++14**  
- CMake >= 3.11  


### Dependencies

#### External

- [opencv](https://opencv.org/) for image decoding & resizing  
- [ffmpeg](https://ffmpeg.org/) for video decoding & frame extracting  
- [fftw](https://www.fftw.org/) for discrete cosine transform (DCT)  
- [sqlite3](https://www.sqlite.org/) for file hash value caching  
- [spdlog](https://github.com/gabime/spdlog) for logging  

**CentOS**

```bash
sudo yum install opencv-devel ffmpeg-devel fftw-devel sqlite-devel spdlog-devel
```

**Ubuntu**

```bash
sudo apt install libopencv-dev libavformat-dev libavcodec-dev libavdevice-dev libavutil-dev libswscale-dev
sudo apt install libfftw3-dev libsqlite3-dev libspdlog-dev
```

**macOS**

```bash
brew install opencv@4 ffmpeg@5 fftw sqlite spdlog
brew link ffmpeg@5
```

#### Included

- [wavelib](https://github.com/rafat/wavelib) for wavelet decomposition  
- [sqlite_orm](https://github.com/fnc12/sqlite_orm) for file hash value caching  
- [cpptqdm](https://github.com/aminnj/cpptqdm) for tqdm like progress bar  
- [CLI11](https://github.com/CLIUtils/CLI11) for command line parsing  

### Compile

```bash
git clone https://github.com/helloall1900/vhash.git
cd vhash
make
```

```bash
bin/vhash hash tests/testdata/lena.png
```

### Development

#### Dependencies

- [googletest](https://github.com/google/googletest) for unit testing  
- [google benchmark](https://github.com/google/benchmark) for benchmarking 

**CentOS**

```bash
sudo yum install gtest-devel google-benchmark-devel
```

**Ubuntu**

```bash
sudo apt install libgtest-dev libbenchmark-dev
```

**macOS**

```bash
brew install googletest google-benchmark
```

--------------------------------------------------------------------------

## Features

- Generate hash value of single file or files in directory.  
- Store file's hash value in db cache to speed up hash generation.  
- Find duplicate video or image files in directory.  

--------------------------------------------------------------------------

## Usage

### Hash

> Generating hash for video or image files  

```bash
Usage: vhash hash [OPTIONS] path  

Positionals:  
path TEXT:PATH(existing) REQUIRED file or directory path  

Options:  
-h,--help                   Print this help message and exit  
-e,--ext TEXT ...           file extension filter (i.e. -e mp4,mkv)  
-c,--cache TEXT             cache file or url  
-o,--output TEXT            output file  
-C,--use-cache              use cache  
-r,--recursive              recursively find files  
-P,--no-progress            not print progress bar  
```

```bash
bin/vhash hash -C -o hash.txt some_dir_path
```

### Cache

> Operating on hash cache  

```bash
Usage: vhash cache [OPTIONS] [path]  

Positionals:  
path TEXT                     full file path  

Options:  
-h,--help                     Print this help message and exit  
-c,--cache TEXT               cache file or url  
-f,--find                     find cache item  
-d,--del                      delete cache item  
-C,--clear                    clear all hash cache  
-p,--pure                     pure expired hash cache  
-P,--pure-period INT [604800] pure period in seconds
```

```bash
bin/vhash cache -f some_file_path
```

### Dup

> Finding duplicate video or image files  

```bash
Usage: vhash dup [OPTIONS] [path]  

Positionals:  
path TEXT:PATH(existing)    file or directory path  

Options:  
-h,--help                   Print this help message and exit  
-e,--ext TEXT ...           file extension filter (i.e. -e mp4,mkv)  
-c,--cache TEXT             cache file or url  
-o,--output TEXT            output file  
-C,--use-cache              use cache  
-r,--recursive              recursively find files  
-P,--no-progress            not print progress bar
```

```bash
bin/vhash dup -C -o dup.txt some_dir_path
```

--------------------------------------------------------------------------

## Credits

- [videohash](https://github.com/akamhy/videohash) for video hash.  
- [imagehash](https://github.com/JohannesBuchner/imagehash) for image hash.  
- [fastimagehash](https://github.com/simon987/fastimagehash) for C++ implementation of image hash.   

--------------------------------------------------------------------------

## License

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/helloall1900/vhash/blob/master/LICENSE)

Copyright (c) 2023 Leo. See
[LICENSE](https://github.com/helloall1900/vhash/blob/master/LICENSE) for details.
