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

#include <functional>
#include "internal/scan.h"

namespace vhash {

inline bool builtin_dir_filter(const std::string& dir) {
    static std::unordered_set<std::string> filter = {
            ".git", ".vscode", ".idea", ".github", ".gitlab", "@eaDir", "__pycache__"
    };

    std::string::size_type pos = dir.find_last_of(file_seperator()) + 1;
    std::string folder = dir.substr(pos, dir.length() - pos);
    return filter.find(folder) == filter.end();
}

scanner::scanner(std::string dirname, bool use_builtin_filter):
        dirname(std::move(dirname)), use_builtin_filter(use_builtin_filter), run_background(false) {
}

scanner::~scanner() {
    if (run_background)
        th.join();
}

inline void for_each_internal(const std::string& dirname, const file_filter_t& filter,
                              bool include_subdir, bool collect, bool use_builtin_filter,
                              pairs_t& pairs) {
    auto dir = opendir(dirname.c_str());
    if (!dir)
        return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (0 == strcmp(ent->d_name, "..") || 0 == strcmp(ent->d_name, ".")) {
            continue;
        }

        auto path = std::string(dirname).append({file_seperator()}).append(ent->d_name);
        if (scanner_check_is_folder(path)) {
            if (use_builtin_filter && !builtin_dir_filter(path))
                continue;
            if (include_subdir)
                for_each_internal(path, filter, include_subdir, collect, use_builtin_filter, pairs); // recursive
        } else {
            if (filter(dirname.c_str(), ent->d_name) && collect)
                pairs.emplace_back(std::make_tuple(dirname, std::string(ent->d_name)));
        }
    }
    closedir(dir);
}

pairs_t scanner::for_each(const file_filter_t& filter, bool include_subdir) {
    pairs_t pairs;
    for_each_internal(scanner_abs_path(dirname), filter, include_subdir, true, use_builtin_filter, pairs);
    return pairs;
}

void scanner::for_each_bg(const file_filter_t& filter, bool include_subdir) {
    th = std::thread([&] {
        pairs_t pairs;
        for_each_internal(scanner_abs_path(dirname), filter, include_subdir, false, use_builtin_filter, pairs);
    });
    run_background = true;
}

void scanner::block_wait() {
    if (run_background) {
        th.join();
        run_background = false;
    }
}

}
