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

#ifndef VHASH_INTERNAL_SCAN_H
#define VHASH_INTERNAL_SCAN_H

#include <string>
#include <tuple>
#include <unordered_set>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

namespace vhash {

using file_filter_t = std::function<bool(const char *parent, const char *file)>;
using pairs_t = std::vector<std::tuple<std::string, std::string>>;
using set_t = std::unordered_set<std::string>;

/**
 * Dir scanner
 */
class scanner {
public:
    scanner() = delete;
    scanner(const scanner& other) = delete;
    explicit scanner(std::string dirname=".", bool use_builtin_filter=true);
    ~scanner();

    pairs_t for_each(const file_filter_t& filter, bool include_subdir=true);

    void for_each_bg(const file_filter_t& filter, bool include_subdir=true);
    void block_wait();

private:
    std::thread th;
    std::string dirname;
    bool use_builtin_filter;
    bool run_background;
};

// Windows
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>

inline char file_seperator() {
	return '\\';
}

inline std::string scanner_abs_path(const std::string& path) {
    char resolved_path[8192] = {0};
    _fullpath(resolved_path, path.c_str(), sizeof(resolved_path));
    return resolved_path;
}

// Linux
#else
#include <unistd.h>
#include <dirent.h>

inline char file_seperator() {
    return '/';
}

inline std::string scanner_abs_path(const std::string& path) {
    char resolved_path[PATH_MAX+1] = {0};
    realpath(path.c_str(), resolved_path);
    return resolved_path;
}
#endif

inline bool scanner_check_exists(const std::string& path) {
    struct stat st = {0};
    return stat(path.c_str(), &st) == 0;
}

inline bool scanner_check_is_folder(const std::string& path) {
    struct stat st = {0};
    int rtn = stat(path.c_str(), &st);
    if (rtn) return false;
    return S_ISDIR(st.st_mode);
}

inline bool scanner_check_is_file(const std::string& path) {
    struct stat st = {0};
    int rtn = stat(path.c_str(), &st);
    if (rtn) return false;
    return S_ISREG(st.st_mode);
}

inline int scanner_get_file_info(const std::string& path, int64_t& size, int64_t& mtime) {
    struct stat st = {0};
    int rtn = stat(path.c_str(), &st);
    if (rtn) return rtn;
    size = st.st_size;
    mtime = st.st_mtime;
    return 0;
}

inline std::string scanner_lower_ext(const std::string& file) {
    std::string::size_type pos = file.find_last_of('.') + 1;
    std::string ext = file.substr(pos, file.length() - pos);
    for (auto& it : ext) {
        it = static_cast<char>(tolower(it));
    }
    return ext;
}

inline std::tuple<std::string, std::string> scanner_path_split(const std::string& path) {
    std::string::size_type pos = path.find_last_of(file_seperator()) + 1;
    std::string parent = path.substr(0, pos);
    std::string file = path.substr(pos, path.length() - pos);
    return std::make_tuple(scanner_abs_path(parent), file);
}

inline bool scanner_ext_filter(const set_t& black_set, const set_t& white_set, const std::string& file) {
    std::string ext = scanner_lower_ext(file);
    if (!white_set.empty()) {
        return white_set.find(ext) != white_set.end();
    }
    if (!black_set.empty()) {
        return black_set.find(ext) == black_set.end();
    }
    return true;
}

inline std::string scanner_get_home() {
    char *phome = getenv("HOME");
    if (phome)
        return phome;

    struct passwd pwd = {0};
    struct passwd *result;
    char home[PATH_MAX+1] = {0};
    int rtn = getpwuid_r(getuid(), &pwd, home, sizeof(home), &result);
    if (rtn || !result)
        return "";
    return pwd.pw_dir;
}

inline int scanner_mkdir(const std::string& path, mode_t mode) {
    char file_path[PATH_MAX+2] = {0};
    if (path.empty())
        return 0;

    char sep = file_seperator();
    strncpy(file_path, path.c_str(), sizeof(file_path)-2);
    size_t len = strlen(file_path);
    if (file_path[len-1] != sep) {
        file_path[len] = sep;
        file_path[len+1] = '\0';
    }
    if (access(file_path, F_OK) == 0)
        return 0;

    for (char *p = strchr(file_path + 1, sep); p; p = strchr(p + 1, sep)) {
        *p = '\0';
        if (access(file_path, F_OK)) {
            if (mkdir(file_path, mode) == -1) {
                if (errno != EEXIST) {
                    *p = sep;
                    return -1;
                }
            }
        }
        *p = sep;
    }
    return 0;
}

}

#endif //VHASH_INTERNAL_SCAN_H
