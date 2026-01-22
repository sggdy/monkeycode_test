# H.264到H.265转码器

这是一个使用FFmpeg库将H.264编码的视频文件转码为H.265（HEVC）编码的C++程序。

## 功能特性

- 支持H.264视频文件转码为H.265格式
- 使用FFmpeg API进行视频编解码
- 自动处理像素格式转换
- 支持自定义编码质量和参数
- 实时显示转码进度
- 使用CMake进行项目管理

## 环境要求

- CMake 3.10 或更高版本
- C++11 兼容的编译器（如 Visual Studio、GCC、Clang）
- FFmpeg 库（已配置在 `D:\3rdlibsTms\3rdlibsFfmpeg`）
- FFmpeg必须包含H.265（HEVC）编码器支持

## 项目结构

```
ffmpeg/
├── h264_to_h265.cpp      # H.264到H.265转码程序源文件
├── CMakeLists.txt        # CMake配置文件
└── H264_TO_H265_README.md # 说明文档
```

## 编译步骤

### Windows (使用Visual Studio)

1. 打开命令提示符或PowerShell，进入项目目录：
   ```bash
   cd ffmpeg
   ```

2. 创建构建目录：
   ```bash
   mkdir build
   cd build
   ```

3. 生成Visual Studio项目文件：
   ```bash
   cmake ..
   ```

4. 编译项目：
   ```bash
   cmake --build . --config Release
   ```

   或者使用Visual Studio打开生成的 `.sln` 文件进行编译。

### Linux/macOS

1. 进入项目目录：
   ```bash
   cd ffmpeg
   ```

2. 创建构建目录：
   ```bash
   mkdir build
   cd build
   ```

3. 配置CMake：
   ```bash
   cmake ..
   ```

4. 编译：
   ```bash
   make
   ```

## 使用方法

编译成功后，在 `build` 目录下会生成可执行文件 `h264_to_h265`（Windows下为 `h264_to_h265.exe`）。

### 基本用法

```bash
h264_to_h265 <输入.h264文件> <输出.h265文件>
```

### 示例

```bash
# Windows
h264_to_h265.exe input.h264 output.h265

# Linux/macOS
./h264_to_h265 input.h264 output.h265
```

## 配置说明

### FFmpeg库路径

FFmpeg库路径在 [`CMakeLists.txt`](CMakeLists.txt:9) 中配置：

```cmake
set(FFMPEG_ROOT "D:/3rdlibsTms/3rdlibsFfmpeg")
```

如果您的FFmpeg库安装在其他位置，请修改此路径。

### 依赖的FFmpeg库

程序依赖以下FFmpeg库：
- `avformat` - 格式处理
- `avcodec` - 编解码器
- `avutil` - 工具函数
- `swscale` - 图像缩放和像素格式转换

### 编码参数

在 [`h264_to_h265.cpp`](h264_to_h265.cpp:1) 中可以调整以下编码参数：

```cpp
// 编码预设（可选值：ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow）
av_opt_set(encoderContext->priv_data, "preset", "medium", 0);

// CRF值（0-51，值越小质量越高，文件越大）
av_opt_set(encoderContext->priv_data, "crf", "28", 0);

// GOP大小
encoderContext->gop_size = 10;

// 最大B帧数
encoderContext->max_b_frames = 1;

// 帧率
encoderContext->time_base = AVRational{1, 30};  // 30 FPS
encoderContext->framerate = AVRational{30, 1};
```

## 技术实现

### 核心功能

1. **打开输入文件**：使用 `avformat_open_input()` 打开H.264文件
2. **获取流信息**：使用 `avformat_find_stream_info()` 获取媒体流信息
3. **初始化解码器**：使用 `avcodec_find_decoder()` 和 `avcodec_open2()` 初始化H.264解码器
4. **初始化编码器**：使用 `avcodec_find_encoder()` 和 `avcodec_open2()` 初始化H.265编码器
5. **解码视频帧**：使用 `avcodec_send_packet()` 和 `avcodec_receive_frame()` 解码视频帧
6. **像素格式转换**：使用 `sws_scale()` 转换像素格式（如果需要）
7. **编码视频帧**：使用 `avcodec_send_frame()` 和 `avcodec_receive_packet()` 编码为H.265
8. **写入输出文件**：使用 `av_interleaved_write_frame()` 写入编码后的数据包

### 代码结构

- [`H264ToH265Converter`](h264_to_h265.cpp:10) 类：封装转码逻辑
- [`convert()`](h264_to_h265.cpp:21) 方法：执行转码操作
- [`initDecoder()`](h264_to_h265.cpp:85) 方法：初始化解码器
- [`initEncoder()`](h264_to_h265.cpp:115) 方法：初始化编码器
- [`transcode()`](h264_to_h265.cpp:155) 方法：执行转码循环
- [`cleanup()`](h264_to_h265.cpp:345) 方法：清理资源

## H.264 vs H.265

### H.264 (AVC)
- 成熟的视频编码标准
- 广泛支持
- 相对较低的编码复杂度
- 较高的比特率

### H.265 (HEVC)
- 新一代视频编码标准
- 相同质量下比特率降低约50%
- 更高的编码复杂度
- 更好的压缩效率
- 支持4K/8K超高清视频

## 注意事项

1. 确保FFmpeg库路径正确配置
2. FFmpeg必须包含H.265编码器支持（libx265）
3. 输入文件必须是有效的H.264编码视频文件
4. 转码过程需要较多的CPU资源
5. 转码时间取决于视频长度、分辨率和CPU性能
6. 输出文件扩展名建议使用 `.h265` 或 `.mp4`（如果封装为MP4）

## 故障排除

### 编译错误

如果遇到编译错误，请检查：
1. FFmpeg库路径是否正确
2. CMake版本是否符合要求
3. 编译器是否支持C++11
4. FFmpeg是否包含H.265编码器支持

### 运行时错误

如果遇到运行时错误，请检查：
1. 输入文件是否存在且可读
2. 输出路径是否有写入权限
3. FFmpeg DLL文件是否在系统PATH中（Windows）
4. FFmpeg是否支持H.265编码

### 转码质量问题

如果转码质量不理想，可以调整：
1. CRF值（降低值提高质量）
2. 编码预设（使用slower预设提高质量）
3. GOP大小（增加GOP大小可能提高压缩效率）

## 性能优化建议

1. **硬件加速**：如果支持，可以使用硬件编码器（如NVENC、QuickSync）
2. **多线程**：FFmpeg默认支持多线程编码
3. **编码预设**：根据需求选择合适的预设（速度vs质量）
4. **分辨率调整**：可以同时调整分辨率以进一步减小文件大小

## 示例输出

```
开始转码...
输入文件: input.h264
输出文件: output.h265
解码器初始化成功: h264
编码器初始化成功: hevc
分辨率: 1920x1080
已处理 30 帧...
已处理 60 帧...
已处理 90 帧...
总共处理 120 帧
转码成功！输出文件: output.h265
转码完成！
```

## 许可证

本项目仅供学习和参考使用。

## 作者

MonkeyCode