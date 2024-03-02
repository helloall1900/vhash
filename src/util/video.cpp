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

#include <algorithm>
#include "spdlog/spdlog.h"
#include "internal/util.h"

namespace vhash {

VideoDecoder::VideoDecoder(const std::string& file, double rate, int scaled_rows, int scaled_cols):
    file(file), rate(rate), scaled_rows(scaled_rows), scaled_cols(scaled_cols)  {
    init();
    open();
}

VideoDecoder::~VideoDecoder() {
    av_frame_free(&dec_frame);
    av_frame_free(&scaled_frame);
    avcodec_free_context(&avctx);
    avformat_close_input(&afctx);
    av_free(framebuf);
    sws_freeContext(swsctx);
}

inline void VideoDecoder::init() {
    rows = 0;
    cols = 0;
    frames = 0;

    has_opened = false;
    outfmt = AV_PIX_FMT_BGR24;
    afctx = nullptr;
    avctx = nullptr;
    swsctx = nullptr;
    video_stream = nullptr;
    scaled_frame = nullptr;
    dec_frame = nullptr;
    framebuf = nullptr;
    stream_idx = 0;
    peek_frame_idx = 0;
    video_duration = 0;
    end_of_stream = false;
}

inline void VideoDecoder::open() {
    int rtn;
    // open input
    rtn = avformat_open_input(&afctx, file.c_str(), nullptr, nullptr);
    if(rtn < 0) {
        spdlog::error("couldn't open video stream: {}", file);
        return;
    }

    // retrieve input stream information
    rtn = avformat_find_stream_info(afctx, nullptr);
    if(rtn < 0) {
        spdlog::error("no video stream found in the input: {}", file);
        return;
    }

    // find primary video stream and the codec information
#ifdef FFMPEG5
    const AVCodec* vcodec = nullptr;
#else
    AVCodec* vcodec = nullptr;
#endif
    rtn = av_find_best_stream(afctx, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0);
    if(rtn < 0) {
        spdlog::error("no video stream found in the input: {}", file);
        return;
    }
    stream_idx = rtn;
    video_stream = afctx->streams[stream_idx];
    video_duration = afctx->duration;
    if(video_duration <= 0) {
        spdlog::error("invalid video duration: {}", video_duration);
        return;
    }

    // create context from video codec information
    avctx = avcodec_alloc_context3(vcodec);
    if(!avctx) {
        spdlog::error("unable to allocate video context");
        return;
    }

    rtn = avcodec_parameters_to_context(avctx, video_stream->codecpar);
    if(rtn < 0) {
        spdlog::error("unable to create context from video codec information: {}", file);
        return;
    }

    rtn = avcodec_open2(avctx, vcodec, nullptr);
    if(rtn < 0) {
        spdlog::error("unable to open video stream: {}", file);
        return;
    }

    // set video information data members
    rows   = video_stream->codecpar->height;  // number of rows of each frame
    cols   = video_stream->codecpar->width;   // number of columns of each frame
    frames = video_stream->nb_frames;         // number of frames

    if (scaled_rows == 0 && scaled_cols == 0) {
        scaled_rows = rows;
        scaled_cols = cols;
    } else if (scaled_rows == 0) {
        scaled_rows = std::ceil(scaled_cols * rows * 1.0 / cols);
    } else if (scaled_cols == 0) {
        scaled_cols = std::ceil(scaled_rows * cols * 1.0 / rows);
    }

    swsctx = sws_getCachedContext(
            nullptr, video_stream->codecpar->width, video_stream->codecpar->height, avctx->pix_fmt,
            scaled_cols, scaled_rows, outfmt, SWS_BICUBIC,
            nullptr, nullptr, nullptr);
    if(!swsctx) {
        spdlog::error("failed to allocate scale context");
        return;
    }

    // allocate frame buffer for output
    scaled_frame = av_frame_alloc();
    framebuf = (uint8_t *) av_malloc(av_image_get_buffer_size(outfmt, scaled_cols, scaled_rows, 1) * sizeof(uint8_t));
    av_image_fill_arrays(scaled_frame->data, scaled_frame->linesize, framebuf, outfmt, scaled_cols, scaled_rows, 1);

    // allocate decoding frame
    dec_frame = av_frame_alloc();

    has_opened  = true; // video opened successfully
}

inline int decode(AVCodecContext *ctx, AVFrame *frame, bool *got_frame, AVPacket *pkt) {
    int rtn;
    *got_frame = false;

    rtn = avcodec_send_packet(ctx, pkt);
    if(rtn < 0) {
        return rtn == AVERROR_EOF ? 0 : rtn;
    }

    rtn = avcodec_receive_frame(ctx, frame);
    if(rtn < 0 && rtn != AVERROR(EAGAIN) && rtn != AVERROR_EOF) {
        return rtn;
    }
    if(rtn >= 0) {
        *got_frame = true;
    }
    return 0;
}

int VideoDecoder::read(cv::Mat &mat) {
    int rtn;
    bool got_frame = false;
    if(end_of_stream)
        return false;

    AVPacket *pkt = av_packet_alloc();
    do {
        // read packet from input
        rtn = av_read_frame(afctx, pkt);
        if(rtn < 0) {
            if (rtn != AVERROR_EOF) {
                spdlog::error("failed to read frame: {}", file);
                break;
            }
            end_of_stream = true;
            break;
        } else if(pkt->stream_index != stream_idx) {
            av_packet_unref(pkt);
            continue;
        }

        // decode video frame
        rtn = decode(avctx, dec_frame, &got_frame, pkt);
        if(rtn < 0) {
            spdlog::error("failed to decode frame: {}", file);
            break;
        }
        av_packet_unref(pkt);

        if (got_frame) {
            // scale the decoded frame
            sws_scale(swsctx, dec_frame->data, dec_frame->linesize, 0,
                      dec_frame->height, scaled_frame->data, scaled_frame->linesize);
            // copy data to Matrix object
            mat = cv::Mat(scaled_rows, scaled_cols, CV_8UC3, (void*)framebuf, scaled_frame->linesize[0]);
            break;
        }
    } while(true);

    av_packet_free(&pkt);
    return got_frame;
}

inline int64_t video_start_time(const AVStream *in, double seconds, int64_t max_duration) {
    auto start_time = static_cast<int64_t>(seconds * static_cast<double>(AV_TIME_BASE));
    if (start_time > max_duration) return -1;
    return av_rescale_q(start_time, AV_TIME_BASE_Q, in->time_base);
}

int VideoDecoder::peek(cv::Mat &mat) {
    int rtn;
    bool got_frame = false;
    if(end_of_stream)
        return false;

    int64_t start_time = video_start_time(video_stream, rate * peek_frame_idx, video_duration);
    if (start_time < 0) {
        end_of_stream = true;
        return false;
    }
    if (av_seek_frame(afctx, stream_idx, start_time, AVSEEK_FLAG_BACKWARD) < 0){
        end_of_stream = true;
        return false;
    }

    AVPacket *pkt = av_packet_alloc();
    do {
        // read packet from input
        rtn = av_read_frame(afctx, pkt);
        if(rtn < 0) {
            if (rtn != AVERROR_EOF) {
                spdlog::error("failed to read frame: {}", file);
                break;
            }
            end_of_stream = true;
            break;
        } else if(pkt->stream_index != stream_idx) {
            av_packet_unref(pkt);
            continue;
        }

        // decode video frame
        rtn = decode(avctx, dec_frame, &got_frame, pkt);
        if(rtn < 0) {
            spdlog::error("failed to decode frame: {}", file);
            break;
        }
        av_packet_unref(pkt);

        if (got_frame) {
            // scale the decoded frame
            sws_scale(swsctx, dec_frame->data, dec_frame->linesize, 0,
                      dec_frame->height, scaled_frame->data, scaled_frame->linesize);
            // copy data to Matrix object
            mat = cv::Mat(scaled_rows, scaled_cols, CV_8UC3, (void*)framebuf, scaled_frame->linesize[0]);
            break;
        }
    } while(true);

    av_packet_free(&pkt);
    peek_frame_idx++;
    return got_frame;
}

bool VideoDecoder::is_open() const {
    return has_opened;
}

VideoDominantColor::VideoDominantColor() {
    int i = 0;
    for (int j=0; j<16; j++) map[i + j] = ColorType::R;
    i += 16;
    for (int j=0; j<16; j++) map[i + j] = ColorType::G;
    i += 16;
    for (int j=0; j<16; j++) map[i + j] = ColorType::B;
    i += 16;
    for (int j=0; j<16; j++) map[i + j] = ColorType::L;
}

uint64_t VideoDominantColor::hash(const std::vector<cv::Mat>& images) {
    constexpr int max_map = sizeof(map)/sizeof(map[0]);

    uint64_t hv = 0;
    for (int i=0; i<images.size() && i<max_map; i++) {
        auto color = video_get_dominant_color(images[i], resize, min_percent_diff_of_rgb);
        if (color == map[i])
            hv |= (1 << (max_map - i - 1));
    }
    return hv;
}

std::vector<cv::Mat> video_make_thumb(const std::string& file, double rate, int scaled_rows, int scaled_cols) {
    std::vector<cv::Mat> images;
    VideoDecoder v(file, rate, scaled_rows, scaled_cols);
    if (!v.is_open())
        return images;

    bool got_frame;
    do {
        cv::Mat mat;
        got_frame = v.peek(mat);
        if (got_frame) {
            images.emplace_back(mat.clone());
        }
    } while(got_frame);
    return images;
}

cv::Mat video_make_collage(const std::vector<cv::Mat>& images, int max_image_width) {
    if (images.empty())
        return {};

    // parameters of image
    int image_height = images[0].rows;
    int image_width = images[0].cols;
    int image_type = images[0].type();

    // square shape
    int images_per_row = std::floor(std::sqrt(images.size()));
    double scale = 1.0;
    if (images_per_row * image_width > max_image_width) {
        scale = max_image_width * 1.0 / (images_per_row * image_width);
    }
    int scaled_height = std::ceil(image_height * scale);
    int scaled_width = std::ceil(image_width * scale);
    int number_of_rows = std::ceil(static_cast<double>(images.size()) / images_per_row);

    // create our result matrix
    cv::Mat mat = cv::Mat::zeros(number_of_rows * scaled_height, images_per_row * scaled_width, image_type);
    size_t i = 0;
    int current_height = 0;
    int current_width = 0;
    for (int y = 0; y < number_of_rows; y++) {
        for (int x = 0; x < images_per_row; x++) {
            if (i >= images.size())
                break;
            // get the ROI in our result-image
            cv::Mat rect(mat,
                       cv::Range(current_height, current_height + scaled_height),
                       cv::Range(current_width, current_width + scaled_width));
            // resize and copy the current image to the ROI
            cv::Mat image;
            if (scaled_width != images[i].rows || scaled_height != images[i].cols)
                cv::resize(images[i], image, cv::Size(scaled_width, scaled_height), 0, 0, cv::INTER_AREA);
            else
                image = images[i];
            image.copyTo(rect);
            i++;
            current_width += scaled_width;
        }
        // next line - reset width and update height
        current_width = 0;
        current_height += scaled_height;
    }

    return mat;
}

ColorType video_get_dominant_color(const cv::Mat& mat, int resize, int min_percent_diff_of_rgb) {
    // resize image
    cv::Mat image;
    try {
        cv::resize(mat, image, cv::Size(resize, resize), 0, 0, cv::INTER_AREA);
    } catch (cv::Exception &e) {
        spdlog::error("decode or resize image file with exception: {}", e.what());
        return ColorType::N;
    }

    struct {int b, g, r, l;} counter = {0};
    auto dominant_color_of_pixel = [&counter](const uint8_t pixel[4]) -> ColorType {
        uint8_t b = pixel[0], g = pixel[1], r = pixel[2];
        if (b > r && b > g) {
            counter.b += 1;
            return ColorType::B;
        }
        if (g > r && g > b) {
            counter.g += 1;
            return ColorType::G;
        }
        if (r > g && r > b) {
            counter.r += 1;
            return ColorType::R;
        }

        counter.l += 1;
        return ColorType::L;
    };

    // count BGRA
    int ch = image.channels();
    for (int i=0; i<image.rows; i++) {
        auto* row = image.ptr<unsigned char>(i);
        for (int j=0; j<image.cols*image.channels(); j+=ch) {
            uint8_t pixel[4] = {0}; // BGRA
            for (int c=0; c<ch; c++) {
                pixel[c] = row[j+c];
            }
            dominant_color_of_pixel(pixel);
        }
    }

    // calc image dominant color
    int total_pixels = image.rows * image.cols;
    auto dominant_color_of_image = [&total_pixels, &min_percent_diff_of_rgb, &counter]() -> ColorType {
        if (counter.l >= counter.b && counter.l >= counter.g && counter.l >= counter.r)
            return ColorType::L;

        int mpd = total_pixels * min_percent_diff_of_rgb / 100;
        if ((counter.b - mpd > counter.r) && (counter.b - mpd > counter.g))
            return ColorType::B;
        if ((counter.g - mpd > counter.r) && (counter.g - mpd > counter.b))
            return ColorType::G;
        if ((counter.r - mpd > counter.g) && (counter.r - mpd > counter.b))
            return ColorType::R;

        return ColorType::N;
    };

    return dominant_color_of_image();
}

}
