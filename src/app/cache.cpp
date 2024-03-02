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
#include "vhash_app.h"
#include "vhash_error.h"
#include "internal/cache.h"
#include "internal/scan.h"

namespace vhash {

int cache_cmd(const cache_config& conf) {
    if ((conf.find || conf.del) && conf.path.empty()) {
        spdlog::error("path: should not be empty");
        return VERROR(errors::ERR_PARAM_INVALID);
    }

    // init cache db
    db_cache db(conf.cache_url);
    int rtn = db.init();
    if(rtn) return rtn;

    // operating cache
    auto v = scanner_path_split(conf.path);
    if (conf.find) {
        cache_item key{.parent=std::get<0>(v), .file=std::get<1>(v)};
        auto items = db.get(key);
        for (auto& it : items) {
            std::cout << "FILE: " << conf.path << std::endl;
            std::cout << "HASH: 0x" << it.file_hash << std::endl;
        }
    } else if (conf.del) {
        cache_item key{.parent=std::get<0>(v), .file=std::get<1>(v)};
        rtn = db.del(key);
    } else if (conf.clear) {
        rtn = db.clear();
    } else if (conf.pure) {
        rtn = db.pure(conf.pure_period);
    }
    return rtn;
}

}
