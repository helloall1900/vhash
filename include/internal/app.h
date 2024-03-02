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

#ifndef VHASH_INTERNAL_APP_H
#define VHASH_INTERNAL_APP_H

#include <string>
#include <mutex>
#include <unordered_set>
#include "vhash_hash.h"
#include "internal/cache.h"
#include "internal/scan.h"
#include "internal/util.h"

namespace vhash {

extern std::unordered_set<std::string> images;
extern std::unordered_set<std::string> videos;

inline FileType app_check_file_type(const std::string& path) {
    std::string ext = scanner_lower_ext(path);
    if (images.find(ext) != images.end())
        return FileType::TP_IMAGE;
    if (videos.find(ext) != videos.end())
        return FileType::TP_VIDEO;
    return FileType::TP_OTHER;
}

inline std::unordered_set<std::string> app_generate_ext_set(const std::vector<std::string>& vec) {
    std::unordered_set<std::string> set;
    if (!vec.empty()) {
        for (auto &v : vec) set.emplace(v);
    } else {
        set.insert(images.begin(), images.end());
        set.insert(videos.begin(), videos.end());
    }
    return set;
}

inline uint64_t app_get_file_hash(std::mutex& db_lock, const db_cache& db, const std::string& path, bool use_cache, FileType ft) {
    auto v = scanner_path_split(path);
    cache_item file_info{.parent=std::get<0>(v), .file=std::get<1>(v)};
    scanner_get_file_info(path, file_info.file_size, file_info.file_update_ts);

    if (use_cache) {
        cache_item key {.parent=std::get<0>(v), .file=std::get<1>(v)};
        std::vector<cache_item> item;

        {
            std::lock_guard<std::mutex> lock(db_lock);
            item = db.get(key);
        }

        time_t now;
        time(&now);
        if (!item.empty() &&
            item[0].file_update_ts == file_info.file_update_ts && item[0].file_size == file_info.file_size) {
            return item[0].file_hash;
        }
    }

    hasher h(ft);
    int rtn = h.load(path);
    if (rtn < 0) {
        spdlog::error("load file \"{}\" failed: {}", path, rtn);
        return 0;
    }
    file_info.file_hash = h.hash();

    if (use_cache) {
        std::lock_guard<std::mutex> lock(db_lock);
        db.set(file_info);
    }
    return file_info.file_hash;
}

}

#endif //VHASH_INTERNAL_APP_H
