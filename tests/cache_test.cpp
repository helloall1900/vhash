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

#include <gtest/gtest.h>
#include "internal/cache.h"

using namespace vhash;

TEST(cache, db)
{
    db_cache db("/tmp/test_vhash_db.sqlite");
    int rtn = db.init();
    ASSERT_EQ(rtn, 0);

    // insert
    auto item = cache_item {
        .parent="/home/user/documents",
        .file="demo.jpg",
        .file_size=1024,
        .file_update_ts=1652849680,
        .file_hash=0x12345678,
    };
    rtn = db.set(item);
    ASSERT_EQ(rtn, 0);

    // select
    auto key = cache_item {
            .parent="/home/user/documents",
            .file="demo.jpg",
    };
    auto v = db.get(key);
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(v[0].file_size, 1024);

    // update
    item.file_size = 2048;
    rtn = db.set(item);
    ASSERT_EQ(rtn, 0);
    v = db.get(key);
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(v[0].file_size, 2048);

    // delete
    rtn = db.del(key);
    ASSERT_EQ(rtn, 0);
    v = db.get(key);
    ASSERT_EQ(v.size(), 0);

    // clear
    rtn = db.set(item);
    ASSERT_EQ(rtn, 0);
    rtn = db.clear();
    ASSERT_EQ(rtn, 0);
    v = db.get(key);
    ASSERT_EQ(v.size(), 0);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
