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
#include "internal/imagehash.h"

using namespace vhash;

TEST(imagehash, hashval_ops)
{
    std::string bs = "1111_0001_1110_0010_1101_0011_1100_0100";
    hashval<4> hv;

    int i = 0;
    for (auto b : bs) {
        if (b != '_') {
            hv.set(i, b == '1');
            i += 1;
        }
    }

    hashval<4> hv0;
    hashval<4> hv1(hv);
    EXPECT_FALSE(hv == hv0);
    EXPECT_FALSE(hv < hv0);
    EXPECT_FALSE(hv <= hv0);
    EXPECT_NE(hv, hv0);
    EXPECT_GT(hv, hv0);
    EXPECT_GE(hv, hv0);
    EXPECT_EQ(hv, hv1);
    EXPECT_GE(hv, hv1);
    EXPECT_LE(hv, hv1);

    std::array<uint8_t, 4> arr1 = {0x11, 0x12, 0x13, 0x14};
    hashval<4> hv2;
    hv2.set(arr1);
    EXPECT_EQ(hv2.hex(), "11121314");
    EXPECT_EQ(hv2.size(), 4);
}

TEST(imagehash, hashval1)
{
    std::string bs = "1111_0001";
    hashval<1> hv;

    int i = 0;
    for (auto b : bs) {
        if (b != '_') {
            hv.set(i, b == '1');
            i += 1;
        }
    }

    EXPECT_EQ(hv.bin(), "11110001");
    EXPECT_EQ(hv.hex(), "f1");
    EXPECT_EQ(hv.uint8(), 0xf1);
}

TEST(imagehash, hashval2)
{
    std::string bs = "1111_0001_1110_0010";
    hashval<2> hv;

    int i = 0;
    for (auto b : bs) {
        if (b != '_') {
            hv.set(i, b == '1');
            i += 1;
        }
    }

    EXPECT_EQ(hv.bin(), "1111000111100010");
    EXPECT_EQ(hv.hex(), "f1e2");
    EXPECT_EQ(hv.uint16(), 0xf1e2);
}

TEST(imagehash, hashval4)
{
    std::string bs = "1111_0001_1110_0010_1101_0011_1100_0100";
    hashval<4> hv;

    int i = 0;
    for (auto b : bs) {
        if (b != '_') {
            hv.set(i, b == '1');
            i += 1;
        }
    }

    EXPECT_EQ(hv.bin(), "11110001111000101101001111000100");
    EXPECT_EQ(hv.hex(), "f1e2d3c4");
    EXPECT_EQ(hv.uint32(), 0xf1e2d3c4);
}

TEST(imagehash, hashval8)
{
    std::string bs = "1111_0001_1110_0010_1101_0011_1100_0100_1011_0101_1010_0110_1001_0111_1000_1000";
    hashval<8> hv;

    int i = 0;
    for (auto b : bs) {
        if (b != '_') {
            hv.set(i, b == '1');
            i += 1;
        }
    }

    EXPECT_EQ(hv.bin(), "1111000111100010110100111100010010110101101001101001011110001000");
    EXPECT_EQ(hv.hex(), "f1e2d3c4b5a69788");
    EXPECT_EQ(hv.uint64(), 0xf1e2d3c4b5a69788);
}

TEST(imagehash, ahash)
{
    ahash<8> h;
    h.load("tests/testdata/lena.png");
    auto hv = h.hash();
    EXPECT_NE(hv.uint64(), 0);
}

TEST(imagehash, phash)
{
    phash<8> h;
    h.load("tests/testdata/lena.png");
    auto hv = h.hash();
    EXPECT_NE(hv.uint64(), 0);
}

TEST(imagehash, dhash)
{
    dhash<8> h;
    h.load("tests/testdata/lena.png");
    auto hv = h.hash();
    EXPECT_NE(hv.uint64(), 0);
}

TEST(imagehash, whash)
{
    whash<8> h;
    h.load("tests/testdata/lena.png");
    auto hv = h.hash();
    EXPECT_NE(hv.uint64(), 0);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
