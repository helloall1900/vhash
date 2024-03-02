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

#include <ctime>
#include <utility>
#include "sqlite_orm/sqlite_orm.h"
#include "spdlog/spdlog.h"
#include "vhash_error.h"
#include "internal/cache.h"
#include "internal/scan.h"

namespace vhash {

using namespace sqlite_orm;

db_cache::db_cache(const std::string& db_file): db_file(db_file), storage(nullptr) {
    if (db_file.empty()) {
        std::string s = scanner_get_home();
        s += file_seperator();
        s += ".vhash";
        s += file_seperator();
        s += "vhash_db.sqlite";
        this->db_file = s;
    }
}

int db_cache::init() {
    auto v = scanner_path_split(db_file);
    int rtn = scanner_mkdir(std::get<0>(v), 0755);
    if (rtn) {
        spdlog::error("create folder \"{}\" failed", std::get<0>(v));
        return VERROR(errors::ERR_MK_DIR);
    }
    try {
        storage = std::make_unique<db_storage>(init_storage(db_file));
        storage->sync_schema();
        return 0;
    } catch (std::system_error& e) {
        spdlog::error("init db storage with exception: {}", e.what());
        return VERROR(errors::ERR_INIT_DB);
    }
}

std::vector<cache_item> db_cache::get(const cache_item& key) const {
    try {
        return storage->get_all<cache_item>(
                where(c(&cache_item::parent) == key.parent and
                      c(&cache_item::file) == key.file));
    } catch (std::system_error& e) {
        spdlog::error("select db record with exception: {}", e.what());
        return {};
    }
}

int db_cache::set(cache_item& item) const {
    try {
        time_t now;
        time(&now);
        item.rec_update_ts = now;
        storage->replace(item);
        return 0;
    } catch (std::system_error& e) {
        spdlog::error("insert db record with exception: {}", e.what());
        return VERROR(errors::ERR_INSERT_DB);
    }
}

int db_cache::del(const cache_item& key) const {
    try {
        storage->remove<cache_item>(key.parent, key.file);
        return 0;
    } catch (std::system_error& e) {
        spdlog::error("delete db record with exception: {}", e.what());
        return VERROR(errors::ERR_DELETE_DB);
    }
}

int db_cache::clear() const {
    try {
        storage->remove_all<cache_item>();
        return 0;
    } catch (std::system_error& e) {
        spdlog::error("clear db records with exception: {}", e.what());
        return VERROR(errors::ERR_CLEAR_DB);
    }
}

int db_cache::pure(int64_t period) const {
    time_t now;
    time(&now);
    uint64_t expired = now - period;

    try {
        storage->remove_all<cache_item>(where(c(&cache_item::rec_update_ts) < expired));
        return 0;
    } catch (std::system_error& e) {
        spdlog::error("pure db records with exception: {}", e.what());
        return VERROR(errors::ERR_PURE_DB);
    }
}

}
