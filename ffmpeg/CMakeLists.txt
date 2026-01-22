cmake_minimum_required(VERSION 3.10)
project(TSConverter)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置FFmpeg库路径
set(FFMPEG_ROOT "D:/3rdlibsTms/3rdlibsFfmpeg")

# 设置FFmpeg头文件路径
set(FFMPEG_INCLUDE_DIRS
    "${FFMPEG_ROOT}/include"
)

# 设置FFmpeg库文件路径
set(FFMPEG_LIB_DIRS
    "${FFMPEG_ROOT}/lib"
)

# 设置FFmpeg库文件
set(FFMPEG_LIBRARIES
    avformat
    avcodec
    avutil
    swscale
    swresample
)

# 添加头文件路径
include_directories(${FFMPEG_INCLUDE_DIRS})

# 添加库文件路径
link_directories(${FFMPEG_LIB_DIRS})

# 添加可执行文件
add_executable(ts_to_mp4 ts_to_mp4.cpp)
add_executable(h264_to_h265 h264_to_h265.cpp)

# 链接FFmpeg库
target_link_libraries(ts_to_mp4 ${FFMPEG_LIBRARIES})
target_link_libraries(h264_to_h265 ${FFMPEG_LIBRARIES})

# Windows平台特定设置
if(WIN32)
    # 设置运行时库
    set_property(TARGET ts_to_mp4 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# 打印配置信息
message(STATUS "FFmpeg Root: ${FFMPEG_ROOT}")
message(STATUS "FFmpeg Include Dirs: ${FFMPEG_INCLUDE_DIRS}")
message(STATUS "FFmpeg Lib Dirs: ${FFMPEG_LIB_DIRS}")
message(STATUS "FFmpeg Libraries: ${FFMPEG_LIBRARIES}")