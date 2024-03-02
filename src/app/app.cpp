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

#include "spdlog/spdlog.h"
#include "CLI11.hpp"
#include "vhash_app.h"

namespace vhash {

const char* VHASH_VERSION = "vhash 0.0.1 (20220520)";

int app_run(int argc, char *argv[]) {
    CLI::App app{"Video and image hash tool", "vhash"};
    app.set_version_flag("-v,--version", VHASH_VERSION);
    bool silent = false;
    app.add_flag("-s,--silent", silent, "Run in silent way");

    auto not_empty_checker = [](const std::string& s) -> std::string {
        if (s.empty()) {
            return "should not be empty";
        }
        return "";
    };

    // cache command
    cache_config c_conf;
    auto& c_cmd = *app.add_subcommand("cache", "Operating on hash cache");
    c_cmd.add_option("path", c_conf.path, "full file path");
    c_cmd.add_option("-c,--cache", c_conf.cache_url, "cache file or url");
    c_cmd.add_flag("-f,--find", c_conf.find, "find cache item");
    c_cmd.add_flag("-d,--del", c_conf.del, "delete cache item");
    c_cmd.add_flag("-C,--clear", c_conf.clear, "clear all hash cache");
    c_cmd.add_flag("-p,--pure", c_conf.pure, "pure expired hash cache");
    c_cmd.add_option("-P,--pure-period", c_conf.pure_period, "pure period in seconds")->default_val(7 * 24 * 60 * 60);

    // dup command
    dup_config d_conf;
    auto& d_cmd = *app.add_subcommand("dup", "Finding duplicate video or image files");
    d_cmd.add_option("path", d_conf.path, "file or directory path")->check(CLI::ExistingPath);
    d_cmd.add_option("-e,--ext", d_conf.ext, "file extension filter (i.e. -e mp4,mkv)")->delimiter(',')->check(not_empty_checker);
    d_cmd.add_option("-c,--cache", d_conf.cache_url, "cache file or url")->check(not_empty_checker);
    d_cmd.add_option("-o,--output", d_conf.output, "output file")->check(not_empty_checker);
    d_cmd.add_option("-j,--jobs", d_conf.jobs, "parallel jobs")->check(CLI::NonNegativeNumber)->default_val(0);
    d_cmd.add_flag("-C,--use-cache", d_conf.use_cache, "use cache");
    d_cmd.add_flag("-r,--recursive", d_conf.recursive, "recursively find files");
    d_cmd.add_flag("-P,--no-progress", d_conf.no_progress, "not print progress bar");

    // hash command
    hash_config h_conf;
    auto& h_cmd = *app.add_subcommand("hash", "Generating hash for video or image files");
    h_cmd.add_option("path", h_conf.path, "file or directory path")->check(CLI::ExistingPath)->required();
    h_cmd.add_option("-e,--ext", h_conf.ext, "file extension filter (i.e. -e mp4,mkv)")->delimiter(',')->check(not_empty_checker);
    h_cmd.add_option("-c,--cache", h_conf.cache_url, "cache file or url")->check(not_empty_checker);
    h_cmd.add_option("-o,--output", h_conf.output, "output file")->check(not_empty_checker);
    h_cmd.add_option("-j,--jobs", h_conf.jobs, "parallel jobs")->check(CLI::NonNegativeNumber)->default_val(0);
    h_cmd.add_flag("-C,--use-cache", h_conf.use_cache, "use cache");
    h_cmd.add_flag("-r,--recursive", h_conf.recursive, "recursively find files");
    h_cmd.add_flag("-P,--no-progress", h_conf.no_progress, "not print progress bar");

    CLI11_PARSE(app, argc, argv);
    if (silent) {
        spdlog::set_level(spdlog::level::off);
    }
    if (c_cmd) {
        return cache_cmd(c_conf);
    } else if (d_cmd) {
        return dup_cmd(d_conf);
    } else if (h_cmd) {
        return hash_cmd(h_conf);
    } else {
        std::cout<< app.help() << std::endl;
    }
    return 0;
}

}
