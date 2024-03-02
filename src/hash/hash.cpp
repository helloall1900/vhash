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

#include "vhash_hash.h"
#include "internal/imagehash.h"

namespace vhash {

/**
 * hasher implement
 */
class hasher::hashimpl {
public:
    explicit hashimpl(FileType ft=FileType::TP_IMAGE, HashType ht=HashType::TP_WHASH): dch(0), ft(ft) {
        switch (ht) {
            case HashType::TP_AHASH:
                h = new ahash<8>;
                break;
            case HashType::TP_PHASH:
                h = new phash<8>;
                break;
            case HashType::TP_DHASH:
                h = new dhash<8>;
                break;
            case HashType::TP_WHASH:
                h = new whash<8>;
                break;
            default:
                break;
        }
    }
    ~hashimpl() {
        delete h;
        h = nullptr;
    }

    int load(const std::string& file_path) {
        return h->load(file_path);
    }

    int load(const cv::Mat& mat) {
        return h->load(mat);
    }

    int load(const std::vector<cv::Mat>& images) {
        if (ft == FileType::TP_VIDEO) {
            VideoDominantColor dc;
            dch = dc.hash(images);
        }
        return 0;
    }

    uint64_t hash() {
        uint64_t hv = h->hash().uint64();
        return hv ^ dch;
    }

private:
    imagehash<8> *h;         /* main hash */
    uint64_t dch;            /* domain color hash */
    FileType ft;             /* file type */
};

/**
 * Hasher
 */
hasher::hasher(FileType ft, HashType ht): ft(ft), ht(ht) {
    impl = new hasher::hashimpl(ft, ht);
}

hasher::hasher(hasher&& other) noexcept: ft(other.ft), ht(other.ht), impl(nullptr) {
    *this = std::move(other);
}

hasher::~hasher() {
    delete impl;
    impl = nullptr;
}

hasher& hasher::operator=(hasher&& other) noexcept {
    if (this != &other) {
        delete impl;
        ht = other.ht;
        ft = other.ft;
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

int hasher::load(const std::string& file_path) {
    if (ft == FileType::TP_IMAGE)
        return impl->load(file_path);
    else if (ft == FileType::TP_VIDEO) {
        auto images = vhash::video_make_thumb(file_path);
        if (images.empty()) {
            return VERROR(errors::ERR_MAKE_THUMB);
        }
        auto image = vhash::video_make_collage(images);
        impl->load(images);
        return impl->load(image);
    }
    return 0;
}

uint64_t hasher::hash() {
    return impl->hash();
}

}
