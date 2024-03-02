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

#ifndef VHASH_INTERNAL_CACHE_H
#define VHASH_INTERNAL_CACHE_H

#include <string>
#include "sqlite_orm/sqlite_orm.h"

namespace vhash {

struct cache_item {
    std::string parent;
    std::string file;

    int64_t file_size;
    int64_t file_update_ts;
    int64_t rec_update_ts;

    uint64_t file_hash;
};

class cache {
public:
    cache() = default;
    cache(const cache& other) = delete;
    virtual ~cache() = default;

    virtual int init() = 0;
    virtual std::vector<cache_item> get(const cache_item& key) const = 0;
    virtual int set(cache_item& item) const = 0;
    virtual int del(const cache_item& key) const = 0;
    virtual int clear() const = 0;
    virtual int pure(int64_t period) const = 0;
};

inline auto init_storage(const std::string& db_file) {
    using namespace sqlite_orm;
    return make_storage(db_file,
                        make_table("cache",
                                   make_column("parent", &cache_item::parent),
                                   make_column("file", &cache_item::file),
                                   make_column("file_size", &cache_item::file_size),
                                   make_column("file_update_ts", &cache_item::file_update_ts),
                                   make_column("rec_update_ts", &cache_item::rec_update_ts),
                                   make_column("file_hash", &cache_item::file_hash),
                                   primary_key(&cache_item::parent, &cache_item::file))
    );
}

class db_cache: public cache {
public:
    using cache::cache;

    explicit db_cache(const std::string& db_file);
    db_cache(const db_cache& other) = delete;
    ~db_cache() override = default;

    int init() override;
    std::vector<cache_item> get(const cache_item& key) const override;
    int set(cache_item& item) const override;
    int del(const cache_item& key) const override;
    int clear() const override;
    int pure(int64_t period) const override;

private:
    using db_storage = decltype(init_storage(""));

    std::unique_ptr<db_storage> storage;
    std::string db_file;
};

}

#endif //VHASH_INTERNAL_CACHE_H
