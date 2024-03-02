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
#include "internal/cache.h"

using namespace vhash;

static void BM_cache_db(benchmark::State& state) {
    db_cache db("/tmp/test_vhash_db.sqlite");
    db.init();

    auto item = cache_item {
            .parent="/home/user/documents",
            .file="demo.jpg",
            .file_size=1024,
            .file_update_ts=1652849680,
            .file_hash=0x12345678,
    };
    auto key = cache_item {
            .parent="/home/user/documents",
            .file="demo.jpg",
    };
    for (auto _ : state) {
        db.set(item);
        auto v = db.get(key);
    }
}
BENCHMARK(BM_cache_db);

BENCHMARK_MAIN();
