find_package(PkgConfig REQUIRED)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h REQUIRED)
find_library(avformat avformat)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h REQUIRED)
find_library(avcodec avcodec)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h REQUIRED)
find_library(avdevice avdevice)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h REQUIRED)
find_library(avutil avutil)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h REQUIRED)
find_library(swscale swscale)

set(FFMPEG_LIBS
    ${avformat}
    ${avcodec}
    ${avdevice}
    ${avutil}
    ${swscale}
)

set(FFMPEG_INCLUDE_DIRS
    ${AVFORMAT_INCLUDE_DIR}
    ${AVCODEC_INCLUDE_DIR}
    ${AVDEVICE_INCLUDE_DIR}
    ${AVUTIL_INCLUDE_DIR}
    ${SWSCALE_INCLUDE_DIR}
)
