// Copyright (c) 2022 Leo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <benchmark/benchmark.h>
#include "vhash_hash.h"
#include "internal/util.h"

using namespace vhash;

static void BM_hasher_image(benchmark::State& state) {
    for (auto _ : state){
        hasher h;
        h.load("tests/testdata/lena.png");
        h.hash();
    }
}
BENCHMARK(BM_hasher_image);

static void BM_hasher_image_parallel(benchmark::State& state) {
    ThreadPool pool;
    std::string path = "tests/testdata/lena.png";
    for (auto _ : state){
        std::future<uint64_t> future = pool.commit([&path]() -> uint64_t {
            hasher h(FileType::TP_IMAGE);
            h.load(path);
            return h.hash();
        });
        uint64_t hv = future.get();
    }
}
BENCHMARK(BM_hasher_image_parallel);

static void BM_hasher_video(benchmark::State& state) {
    for (auto _ : state){
        hasher h(FileType::TP_VIDEO);
        h.load("tests/testdata/video.mp4");
        h.hash();
    }
}
BENCHMARK(BM_hasher_video);

static void BM_hasher_video_parallel(benchmark::State& state) {
    ThreadPool pool;
    std::string path = "tests/testdata/video.mp4";
    for (auto _ : state){
        std::future<uint64_t> future = pool.commit([&path]() -> uint64_t {
            hasher h(FileType::TP_VIDEO);
            h.load(path);
            return h.hash();
        });
        uint64_t hv = future.get();
    }
}
BENCHMARK(BM_hasher_video_parallel);

BENCHMARK_MAIN();
