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

#include <iostream>
#include "spdlog/spdlog.h"
#include "tqdm.h"
#include "vhash_error.h"
#include "vhash_hash.h"
#include "vhash_app.h"
#include "internal/app.h"
#include "internal/cache.h"
#include "internal/scan.h"
#include "internal/util.h"

namespace vhash {

int dup_cmd(const dup_config& conf) {
    if (!scanner_check_exists(conf.path)) {
        spdlog::error("path \"{}\" not exists", conf.path);
        return VERROR(errors::ERR_NOT_EXISTS);
    }

    if (!scanner_check_is_folder(conf.path)) {
        spdlog::error("path \"{}\" is not folder", conf.path);
        return VERROR(errors::ERR_PARAM_INVALID);
    }

    // init output writer
    FileWriter fw(conf.output);

    // init cache db
    db_cache db(conf.cache_url);
    if (conf.use_cache) {
        int rtn = db.init();
        if(rtn) return rtn;
    }

    // generate file hash
    std::vector<std::string> files;
    set_t black_set;
    set_t white_set = app_generate_ext_set(conf.ext);
    scanner scan(conf.path);
    scan.for_each([&files, &black_set, &white_set](const char *parent, const char *file) -> bool {
        if (!scanner_ext_filter(black_set, white_set, file))
            return false;

        std::string file_path = parent;
        file_path += file_seperator();
        file_path += file;
        files.emplace_back(file_path);

        return false; // file has been processed, not add to results
    }, conf.recursive);

    tqdm bar;
    bool has_progress = !conf.no_progress && !conf.output.empty();

    std::mutex db_lock;
    std::mutex map_lock;

    ThreadPool pool(conf.jobs);
    std::unordered_map<uint64_t, std::vector<std::string>> map;
    std::atomic<int> completed{0};
    int total = static_cast<int>(files.size());
    for (auto& file : files) {
        if (has_progress) bar.progress(completed.load(), total);

        while (pool.idle_count() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait 100ms
        }

        pool.commit([&conf, &db, &file, &map, &db_lock, &map_lock, &completed]() {
            FileType ft = app_check_file_type(file);
            if (ft == FileType::TP_OTHER) {
                completed ++;
                return;
            }

            uint64_t hv = app_get_file_hash(db_lock, db, file, conf.use_cache, ft);

            {
                std::lock_guard<std::mutex> lock(map_lock);
                auto it = map.find(hv);
                if (it != map.end()) {
                    it->second.emplace_back(file);
                } else {
                    map.emplace(hv, std::vector<std::string>{std::move(file)});
                }
            }
            completed ++;
        });
    }

    // wait all tasks finished
    while (completed.load() < total) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait 100ms
    }
    if (has_progress) bar.finish();

    // find duplication by hash
    for (auto& m: map) {
        if (m.second.size() > 1) {
            fw << "HASH: 0x" << std::hex << m.first << "\n";
            for (auto& item: m.second) {
                fw << "FILE: " << item << "\n";
            }
            fw << "\n";
        }
    }

    return 0;
}

}
