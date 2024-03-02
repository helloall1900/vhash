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

#ifndef VHASH_APP_H
#define VHASH_APP_H

#include <string>
#include <vector>

namespace vhash {
/**
 * config for cache cmd
 */
struct cache_config {
    std::string path;
    std::string cache_url;
    int64_t pure_period;
    bool find;
    bool del;
    bool clear;
    bool pure;

    cache_config():pure_period(0), find(false), del(false), clear(false), pure(false) {}
};

/**
 * config for dup cmd
 */
struct dup_config {
    std::string path;
    std::vector<std::string> ext;
    std::string cache_url;
    std::string output;
    int jobs;
    bool use_cache;
    bool recursive;
    bool no_progress;

    dup_config(): jobs(0), use_cache(false), recursive(false), no_progress(false) {}
};

/**
 * config for hash cmd
 */
struct hash_config {
    std::string path;
    std::vector<std::string> ext;
    std::string cache_url;
    std::string output;
    int jobs;
    bool use_cache;
    bool recursive;
    bool no_progress;

    hash_config(): jobs(0), use_cache(false), recursive(false), no_progress(false) {}
};

int cache_cmd(const cache_config& conf);
int dup_cmd(const dup_config& conf);
int hash_cmd(const hash_config& conf);
int app_run(int argc, char *argv[]);

}

#endif //VHASH_APP_H
