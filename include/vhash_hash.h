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

#ifndef VHASH_HASH_H
#define VHASH_HASH_H

#include <memory>
#include <string>
#include <vector>

namespace vhash {

/**
 * Hash Type
 */
enum class HashType {
    TP_AHASH,
    TP_PHASH,
    TP_DHASH,
    TP_WHASH,
};

/**
 * File Type
 */
enum class FileType {
    TP_IMAGE,
    TP_VIDEO,
    TP_OTHER,
};

/**
 * Hasher
 */
class hasher {
public:
    explicit hasher(FileType ft=FileType::TP_IMAGE, HashType ht=HashType::TP_WHASH);
    hasher(const hasher& other) = delete;
    hasher(hasher&& other) noexcept;
    ~hasher();

    hasher& operator=(const hasher& other) = delete;
    hasher& operator=(hasher&& other) noexcept;

    int load(const std::string& file_path);

    uint64_t hash();

private:
    class hashimpl;
    hashimpl *impl;

    HashType ht;
    FileType ft;
};

}

#endif //VHASH_HASH_H
