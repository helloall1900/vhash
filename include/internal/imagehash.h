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

#ifndef VHASH_INTERNAL_IMAGEHASH_H
#define VHASH_INTERNAL_IMAGEHASH_H

#include <string>
#include <array>
#include <utility>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <fftw3.h> // for fft
#include <wavelib.h> // for wavelet
#include <spdlog/spdlog.h>
#include "internal/util.h"
#include "vhash_error.h"
#include "vhash_hash.h"

namespace vhash {

/**
 * Hash value
 */
template<size_t N>
class hashval;

template<size_t N>
std::ostream& operator<<(std::ostream& os, const hashval<N>& hv);

template<size_t N=8>
class hashval {
public:
    hashval(): v({}) {}
    hashval(const hashval& other): v(other.v) {}
    hashval(hashval&& other) noexcept: v(std::move(other.v)) {}

    int set(int index, bool one) noexcept {
        if (index >= v.size() * 8)
            return VERROR(errors::ERR_OUT_OF_RANGE);

        int shift = 8 - index%8 - 1;
        if (one)
            v[index/8] |= 1 << shift;
        else
            v[index/8] &= ~(1 << shift);
        return 0;
    }

    int set(const std::array<uint8_t, N>& arr) noexcept {
        v = arr;
        return 0;
    }

    size_t size() const noexcept {
        return N;
    }

    hashval& operator=(const hashval& other) {
        if (this != &other)
            v = other.v;
        return *this;
    }

    hashval& operator=(hashval&& other) noexcept {
        if (this != &other)
            v = std::move(other.v);
        return *this;
    }

    hashval& operator~() noexcept {
        for (int i=0; i<v.size(); i++) {
            v[i] = ~v[i];
        }
        return *this;
    }

    hashval& operator&(hashval&& other) noexcept {
        for (int i=0; i<v.size(); i++) {
            v[i] &= other.v[i];
        }
        return *this;
    }

    hashval& operator|(hashval&& other) noexcept {
        for (int i=0; i<v.size(); i++) {
            v[i] |= other.v[i];
        }
        return *this;
    }

    hashval& operator^(hashval&& other) noexcept {
        for (int i=0; i<v.size(); i++) {
            v[i] ^= other.v[i];
        }
        return *this;
    }

    bool operator==(const hashval& other) const noexcept {
        return v == other.v;
    }

    bool operator!=(const hashval& other) const noexcept {
        return v != other.v;
    }

    bool operator>(const hashval& other) const noexcept {
        return v > other.v;
    }

    bool operator<(const hashval& other) const noexcept {
        return v < other.v;
    }

    bool operator>=(const hashval& other) const noexcept {
        return v >= other.v;
    }

    bool operator<=(const hashval& other) const noexcept {
        return v <= other.v;
    }

    std::string hex() const noexcept {
        std::string s;
        char hex[2+1] = {0};
        for (auto vi: v) {
            snprintf(hex, sizeof(hex), "%02x", vi);
            s += hex;
        }
        return s;
    }

    std::string bin() const noexcept {
        std::string s;
        char bin[8+1] = {0};
        auto byte2bin = [&](uint8_t u8) {
            for (int i=0; i<8; ++i) {
                bin[8-i-1] = static_cast<char>((u8 & 0x1) + '0');
                u8 >>= 1;
            }
        };
        for (auto vi: v) {
            byte2bin(vi);
            s += bin;
        }
        return s;
    }

    uint8_t  uint8() const noexcept;
    uint16_t uint16() const noexcept;
    uint32_t uint32() const noexcept;
    uint64_t uint64() const noexcept;

    friend std::ostream& operator<< <>(std::ostream& os, const hashval& hv);

private:
    std::array<uint8_t, N> v;
};

template<>
inline uint8_t hashval<1>::uint8() const noexcept {
    return v[0];
}

template<>
inline uint16_t hashval<2>::uint16() const noexcept {
    uint16_t val = 0;
    for (auto vi : v) {
        val <<= 8;
        val += vi;
    }
    return val;
}

template<>
inline uint32_t hashval<4>::uint32() const noexcept {
    uint32_t val = 0;
    for (auto vi : v) {
        val <<= 8;
        val += vi;
    }
    return val;
}

template<>
inline uint64_t hashval<8>::uint64() const noexcept {
    uint64_t val = 0;
    for (auto vi : v) {
        val <<= 8;
        val += vi;
    }
    return val;
}

template<size_t N>
inline std::ostream& operator<<(std::ostream& os, const hashval<N>& hv) {
    if (!hv.v.empty()) os << "0x" << hv.hex();
    return os;
}

/**
 * Image Hash base class
 */
template<size_t N=8>
class imagehash {
public:
    imagehash() = default;
    imagehash(const imagehash& other) = delete;
    virtual ~imagehash() = default;

    int load(const std::string& file_path){
        FileLoader loader(file_path, "rb");
        if (!loader.is_open()) {
            spdlog::error("open file {} failed", file_path);
            return VERROR(errors::ERR_OPEN_FILE);
        }

        std::vector<uint8_t> data;
        loader.read(data);
        try {
            void *img_data = &data[0];
            int img_len = data.size();
            image = cv::imdecode(cv::Mat(1, img_len, CV_8UC1, img_data), cv::IMREAD_GRAYSCALE);
        } catch (cv::Exception &e) {
            spdlog::error("decode image file with exception: {}", e.what());
            return VERROR(errors::ERR_DECODE_IMAGE);
        }
        return data.size();
    }

    int load(const cv::Mat& mat){
        try {
            cv::cvtColor(mat, image, cv::COLOR_BGR2GRAY);
        } catch (cv::Exception &e) {
            spdlog::error("convert image to gray with exception: {}", e.what());
            return VERROR(errors::ERR_DECODE_IMAGE);
        }
        return image.cols * image.rows;
    }

    virtual hashval<N> hash() = 0;

protected:
    cv::Mat image;
};

/**
 * Average Hash computation
 * Implementation follows http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html
 */
template<size_t N=8>
class ahash : public imagehash<N> {
public:
    using imagehash<N>::imagehash;

    static_assert(N >= 2, "Hash size must be greater than or equal to 2");

    hashval<N> hash() override {
        hashval<N> hv;

        cv::Mat im;
        try {
            cv::resize(this->image, im, cv::Size(N, N), 0, 0, cv::INTER_AREA);
        } catch (cv::Exception &e) {
            spdlog::error("decode or resize image file with exception: {}", e.what());
            return hv;
        }

        auto avg = static_cast<unsigned char>(cv::mean(im).val[0]);
        unsigned char *img_data = im.data;
        int img_len = im.rows * im.cols;
        for (int i=0; i<img_len; ++i) {
            hv.set(i, img_data[i] > avg);
        }

        return hv;
    }
};

/**
 * Perceptual Hash computation
 * Implementation follows http://www.hackerfactor.com/blog/index.php?/archives/432-Looks-Like-It.html
 */
template<size_t N=8>
class phash : public imagehash<N> {
public:
    using imagehash<N>::imagehash;
    explicit phash(int high_freq_factor=4): imagehash<N>::imagehash(),high_freq_factor(high_freq_factor) {}

    static_assert(N >= 2, "Hash size must be greater than or equal to 2");

    hashval<N> hash() override {
        hashval<N> hv;

        int img_size = high_freq_factor * N;
        cv::Mat im;
        try {
            cv::resize(this->image, im, cv::Size(img_size, img_size), 0, 0, cv::INTER_AREA);
        } catch (cv::Exception &e) {
            spdlog::error("decode or resize image file with exception: {}", e.what());
            return hv;
        }

        Array<double> pixels(img_size * img_size);
        unsigned char *img_data = im.data;
        int img_len = im.rows * im.cols;
        for (int i = 0; i < img_len; ++i) {
            pixels[i] = static_cast<double>(img_data[i]) / 255.0;
        }

        Array<double> dct(img_size * img_size);
        fftw_plan plan = fftw_plan_r2r_2d(
                img_size, img_size,
                pixels.data(), dct.data(),
                FFTW_REDFT10, FFTW_REDFT10, // DCT-II
                FFTW_ESTIMATE
        );
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        std::array<double, N * N> dct_lowfreq;
        std::array<double, N * N> dct_tosort;

        int index_low = 0;
        int index = 0;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                dct_lowfreq[index_low] = dct[index];
                dct_tosort[index_low] = dct[index];
                index_low += 1;
                index += 1;
            }
            index += (img_size - N);
        }

        auto median = [](std::array<double, N * N>& arr) -> double {
            std::sort(arr.begin(), arr.end());
            size_t len = arr.size();
            return (arr[(len+1)/2 - 1] + arr[len / 2]) / 2.0;
        };

        double med = median(dct_tosort);
        for (int i=0; i<dct_lowfreq.size(); ++i) {
            hv.set(i, dct_lowfreq[i] > med);
        }

        return hv;
    }

private:
    int high_freq_factor;
};

/**
 * Difference Hash computation
 * Implementation follows http://www.hackerfactor.com/blog/index.php?/archives/529-Kind-of-Like-That.html
 */
template<size_t N=8>
class dhash : public imagehash<N> {
public:
    using imagehash<N>::imagehash;

    static_assert(N >= 2, "Hash size must be greater than or equal to 2");

    hashval<N> hash() override {
        hashval<N> hv;

        cv::Mat im;
        try {
            cv::resize(this->image, im, cv::Size(N+1, N), 0, 0, cv::INTER_AREA);
        } catch (cv::Exception &e) {
            spdlog::error("decode or resize image file with exception: {}", e.what());
            return hv;
        }

        int index = 0;
        for (int i=0; i<im.rows; ++i) {
            unsigned char *pixel = im.ptr(i);
            for (int j=1; j<im.cols; ++j) {
                hv.set(index++, pixel[j] > pixel[j - 1]);
            }
        }

        return hv;
    }
};

/**
 * Wavelet Hash computation
 * Implementation based on https://www.kaggle.com/c/avito-duplicate-ads-detection/
 */
template<size_t N=8>
class whash : public imagehash<N> {
public:
    using imagehash<N>::imagehash;
    explicit whash(std::string mode="haar", int img_scale=0, bool remove_max_haar_ll=true):
        imagehash<N>::imagehash(),
        mode(std::move(mode)), img_scale(img_scale), remove_max_haar_ll(remove_max_haar_ll)  {}

    static_assert(N >= 2, "Hash size must be greater than or equal to 2");
    static_assert((N & (N-1)) == 0, "Hash size should be power of 2");

    hashval<N> hash() override {
        hashval<N> hv;

        if (mode != "haar" && mode != "db4") {
            spdlog::error("mode should be haar or db4");
            return hv;
        }

        if (img_scale != 0) {
            if ((img_scale & (img_scale - 1)) != 0) {
                spdlog::error("img_scale should be power of 2");
                return hv;
            }
            if (img_scale < N) {
                spdlog::error("img_scale should greater than or equal to hash size");
                return hv;
            }
        }

        if (img_scale == 0) {
            int image_natural_scale = static_cast<int>(pow(2, static_cast<int>(log2(MIN(this->image.rows, this->image.cols)))));
            img_scale = MAX(image_natural_scale, N);
        }

        int ll_max_level = static_cast<int>(log2(img_scale));
        int level = static_cast<int>(log2(N));
        int dwt_level = ll_max_level - level;
        if (dwt_level < 1)
            dwt_level = 1;

        cv::Mat im;
        try {
            cv::resize(this->image, im, cv::Size(img_scale, img_scale), 0, 0, cv::INTER_AREA);
        } catch (cv::Exception &e) {
            spdlog::error("resize image file with exception: {}", e.what());
            return hv;
        }

        Array<double> pixels(img_scale * img_scale);
        unsigned char *img_data = im.data;
        int img_len = im.cols * im.rows;
        for (int i = 0; i < img_len; i++) {
            pixels[i] = static_cast<double>(img_data[i]) / 255.0;
        }

        // remove low level frequency LL(max_ll) using haar filter
        if (remove_max_haar_ll) {
            wave_object w_haar_tmp = wave_init("haar");
            double img_scale_tmp = static_cast<double>(img_scale) / static_cast<double>(w_haar_tmp->filtlength - 1);
            int max_level_tmp = static_cast<int>(log(img_scale_tmp) / log(2.0));
            ll_max_level = MIN(ll_max_level, max_level_tmp);

            wt2_object wt_haar_tmp = wt2_init(w_haar_tmp, "dwt", img_scale, img_scale, ll_max_level);
            double *coeffs = dwt2(wt_haar_tmp, pixels.data());
            coeffs[0] = 0;
            idwt2(wt_haar_tmp, coeffs, pixels.data());

            wt2_free(wt_haar_tmp);
            wave_free(w_haar_tmp);
            free(coeffs);
        }

        wave_object w = wave_init(mode.c_str());
        wt2_object wt = wt2_init(w, "dwt", img_scale, img_scale, dwt_level);
        double *coeffs = dwt2(wt, pixels.data());

        std::array<double, N * N> coeffs_tosort;
        for (int i=0; i<N * N; i++) coeffs_tosort[i] = coeffs[i];

        auto median = [](std::array<double, N * N>& arr) -> double {
            std::sort(arr.begin(), arr.end());
            size_t len = arr.size();
            return (arr[(len+1)/2 - 1] + arr[len / 2]) / 2.0;
        };

        double med = median(coeffs_tosort);
        for (int i=0; i<coeffs_tosort.size(); ++i) {
            hv.set(i, coeffs[i] > med);
        }

        wt2_free(wt);
        wave_free(w);
        free(coeffs);

        return hv;
    }

private:
    std::string mode;
    int img_scale;
    bool remove_max_haar_ll;
};

}

#endif //VHASH_INTERNAL_IMAGEHASH_H
