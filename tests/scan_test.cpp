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
#include "internal/scan.h"

using namespace vhash;

TEST(scan, ext_filter)
{
    bool valid;

    // black set
    set_t black = {"jpg", "png", "jpeg", "bmp"};
    valid = scanner_ext_filter(black, set_t{}, "demo.jpg");
    EXPECT_EQ(valid, false);
    valid = scanner_ext_filter(black, set_t{}, "demo.JPG");
    EXPECT_EQ(valid, false);
    valid = scanner_ext_filter(black, set_t{}, "demo.txt");
    EXPECT_EQ(valid, true);

    // white set
    set_t white = {"jpg", "png", "jpeg", "bmp"};
    valid = scanner_ext_filter(set_t{}, white, "demo.jpg");
    EXPECT_EQ(valid, true);
    valid = scanner_ext_filter(set_t{}, white, "demo.JPG");
    EXPECT_EQ(valid, true);
    valid = scanner_ext_filter(set_t{}, white, "demo.txt");
    EXPECT_EQ(valid, false);
}

TEST(scan, home)
{
    auto home = scanner_get_home();
    EXPECT_NE(home.size(), 0);
}

TEST(scan, for_each)
{
    scanner s("./tests");
    auto pairs = s.for_each([](const char *parent, const char *file) ->bool {
        set_t black;
        set_t white = {"cpp"};
        return scanner_ext_filter(black, white, file);
    }, true);

    ASSERT_NE(pairs.size(), 0);
    for (auto& it : pairs) {
        std::string ext = scanner_lower_ext(std::get<1>(it));
        EXPECT_EQ(ext, "cpp");
    }
}

TEST(scan, for_each_bg)
{
    scanner s("./tests");
    pairs_t pairs;
    s.for_each_bg([&pairs](const char *parent, const char *file) ->bool {
        set_t black;
        set_t white = {"cpp"};
        bool valid = scanner_ext_filter(black, white, file);
        if (valid)
            pairs.push_back(std::make_tuple(std::string(parent), std::string(file)));
        return valid;
    }, true);
    s.block_wait();

    ASSERT_NE(pairs.size(), 0);
    for (auto& it : pairs) {
        std::string ext = scanner_lower_ext(std::get<1>(it));
        EXPECT_EQ(ext, "cpp");
    }
}

TEST(scan, path_split)
{
    auto v = scanner_path_split("./tests/scan_test.cpp");
    EXPECT_NE(std::get<0>(v), "");
    EXPECT_EQ(std::get<1>(v), "scan_test.cpp");
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
