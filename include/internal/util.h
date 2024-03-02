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

#ifndef VHASH_INTERNAL_UTIL_H
#define VHASH_INTERNAL_UTIL_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>
#include <sys/stat.h>
#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>
#include <thread>
#include <functional>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavdevice/avdevice.h>
};
#include <opencv2/opencv.hpp>
#include "vhash_error.h"

namespace vhash {

/**
 * File loader
 */
class FileLoader {
public:
    FileLoader() : file(nullptr), length(0), error(false) {};
    FileLoader(const FileLoader& other) = delete;
    FileLoader(const std::string& file_path, const std::string& mode);
    ~FileLoader();

    bool is_open() const;

    size_t size() const;

    int read(std::vector<uint8_t>& vec);

private:
    FILE *file;
    size_t length;
    bool error;
};

/**
 * File writer
 * Write to stdout if file_path is empty
 */
class FileWriter {
public:
    explicit FileWriter(const std::string& file_path="");
    FileWriter(const FileWriter& other) = delete;
    ~FileWriter();

    bool is_open() const;

    template<typename T>
    FileWriter& operator<<(const T & args) {
        if (is_cout) {
            (std::cout << args);
        } else {
            (of << args);
        }
        return *this;
    }

private:
    std::ofstream of;
    bool is_cout;
    bool error;
};

/**
 * Array
 */
template<typename T>
class Array {
public:
    explicit Array(size_t sz=0) {
        ptr = sz == 0 ? nullptr : new T[sz];
        size = sz;
    }

    Array(const Array& other) = delete;

    Array(Array&& other) noexcept: Array(0) {
        *this = std::move(other);
    }

    ~Array() {
        delete[]ptr;
        ptr = nullptr;
    }

    Array& operator=(const Array& other) = delete;

    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            delete []ptr;
            ptr = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }

        return *this;
    }

    T& operator[](int index) {
        if (index >= size)
            throw std::out_of_range("index out of range");
        return ptr[index];
    }

    T *data() const noexcept {
        return ptr;
    }

    T *begin() const noexcept {
        return ptr;
    }

    T *end() const noexcept {
        return ptr + size;
    }

    size_t len() const noexcept {
        return size;
    }

private:
    T *ptr;
    size_t size;
};

/**
 * Thread pool
 */
#ifndef THREADPOOL_MAX_NUM
#define THREADPOOL_MAX_NUM  256
#endif

class ThreadPool {
public:
    using Task = std::function<void()>;     // task type

    explicit ThreadPool(size_t size=0) {
        if (size == 0) size = std::thread::hardware_concurrency();
        if (size == 0) size = 8;
        add_threads(size);
    }

    ~ThreadPool() {
        is_run = false;
        cond.notify_all();
        for (std::thread& thread: pool) {
            if (thread.joinable())
                thread.join();
        }
    }

    // commit a task
    // call .get() to wait and get return value
    template<class F, class... Args>
    auto commit(F&& f, Args&& ... args) -> std::future<decltype(f(args...))> {
        if (!is_run)
            throw std::runtime_error("commit on a stopped thread pool");

        // bind function and arguments
        using rtn_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<rtn_type()>> (
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        // generate future
        std::future<rtn_type> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock_gd{lock};
            // add task to queue
            tasks.emplace([task]() {
                (*task)();
            });
        }

        // notify one thread
        cond.notify_one();
        return future;
    }

    // idle threads num
    int idle_count() const { return idle_num.load(); }

    // total threads num
    int pool_size() const { return static_cast<int>(pool.size()); }

private:
    void add_threads(unsigned short size) {
        for (; pool.size() < THREADPOOL_MAX_NUM && size > 0; --size) {
            // thread routine
            pool.emplace_back([this] {
                while (is_run.load()) {
                    Task task;
                    {
                        std::unique_lock <std::mutex> uni_lock{lock};
                        // wait new task
                        cond.wait(uni_lock, [this] {
                            return !is_run.load() || !tasks.empty();
                        });
                        if (!is_run.load() && tasks.empty())
                            return;
                        // pop a task
                        task = move(tasks.front());
                        tasks.pop();
                    }

                    // run task
                    idle_num--;
                    task();
                    idle_num++;
                }
            });
            idle_num++;
        }
    }

    std::vector<std::thread> pool;          // thread pool
    std::queue<Task> tasks;                 // task queue
    std::mutex lock;                        // lock
    std::condition_variable cond;           // condition variable
    std::atomic<bool> is_run{true};      // thread pool is running
    std::atomic<int> idle_num{0};        // idle threads num
};

/**
 * Video decoder
 */
class VideoDecoder {
public:
    explicit VideoDecoder(const std::string& file, double rate=1.0, int scaled_rows=0, int scaled_cols=0);
    ~VideoDecoder();

    int read(cv::Mat &mat);
    int peek(cv::Mat &mat);
    bool is_open() const;

    int     rows;               // number of rows
    int     cols;               // number of columns
    int64_t frames;             // total number of frames

private:
    void init();                // class member initializer
    void open();                // open video file

    std::string file;           // input video file
    double rate;                // one frame per rate second
    int scaled_rows;            // scaled frame rows (height)
    int scaled_cols;            // scaled frame cols (width)

    bool has_opened;            // indicates if input is successfully has_opened or not
    AVPixelFormat outfmt;       // output pixel format
    AVFormatContext *afctx;     // input format context
    AVCodecContext *avctx;      // input video codec context
    SwsContext *swsctx;         // scaling context
    AVStream *video_stream;     // a single video stream
    AVFrame *scaled_frame;      // a single scaled frame
    AVFrame *dec_frame;         // a single decoded frame
    uint8_t *framebuf;          // frame buffer
    int stream_idx;             // index of the video stream in the input
    int peek_frame_idx;         // index of peeking video frame
    int64_t video_duration;     // video duration in AV_TIME_BASE
    bool end_of_stream;         // indicates end of input stream
};

/**
 * Video Dominant Color
 */
enum class ColorType {
    R,   /* red */
    G,   /* green */
    B,   /* blue */
    L,   /* gray */
    N,   /* unknown */
};

class VideoDominantColor {
public:
    VideoDominantColor();

    uint64_t hash(const std::vector<cv::Mat>& images);

private:
    int resize = 16;
    int min_percent_diff_of_rgb = 10;
    ColorType map[64];
};

std::vector<cv::Mat> video_make_thumb(const std::string& file, double rate=1.0, int scaled_rows=144, int scaled_cols=144);
cv::Mat video_make_collage(const std::vector<cv::Mat>& images, int max_image_width=1024);
ColorType video_get_dominant_color(const cv::Mat& image, int resize=16, int min_percent_diff_of_rgb=10);

}

#endif //VHASH_INTERNAL_UTIL_H
