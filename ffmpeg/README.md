# TS转MP4转换器

这是一个使用FFmpeg库将TS（MPEG-TS）格式视频文件转换为MP4格式的C++程序。

## 功能特性

- 支持本地TS文件转换为MP4文件
- 使用FFmpeg API进行视频格式转换
- 使用CMake进行项目管理
- 自动处理时间戳转换
- 支持多流（视频、音频等）转换

## 环境要求

- CMake 3.10 或更高版本
- C++11 兼容的编译器（如 Visual Studio、GCC、Clang）
- FFmpeg 库（已配置在 `D:\3rdlibsTms\3rdlibsFfmpeg`）

## 项目结构

```
ffmpeg/
├── ts_to_mp4.cpp      # 主程序源文件
├── CMakeLists.txt     # CMake配置文件
└── README.md          # 说明文档
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

编译成功后，在 `build` 目录下会生成可执行文件 `ts_to_mp4`（Windows下为 `ts_to_mp4.exe`）。

### 基本用法

```bash
ts_to_mp4 <输入.ts文件> <输出.mp4文件>
```

### 示例

```bash
# Windows
ts_to_mp4.exe input.ts output.mp4

# Linux/macOS
./ts_to_mp4 input.ts output.mp4
```

## 配置说明

### FFmpeg库路径

FFmpeg库路径在 [`CMakeLists.txt`](CMakeLists.txt:13) 中配置：

```cmake
set(FFMPEG_ROOT "D:/3rdlibsTms/3rdlibsFfmpeg")
```

如果您的FFmpeg库安装在其他位置，请修改此路径。

### 依赖的FFmpeg库

程序依赖以下FFmpeg库：
- `avformat` - 格式处理
- `avcodec` - 编解码器
- `avutil` - 工具函数
- `swscale` - 图像缩放
- `swresample` - 音频重采样

## 技术实现

### 核心功能

1. **打开输入文件**：使用 `avformat_open_input()` 打开TS文件
2. **获取流信息**：使用 `avformat_find_stream_info()` 获取媒体流信息
3. **创建输出上下文**：使用 `avformat_alloc_output_context2()` 创建MP4输出上下文
4. **复制流参数**：使用 `avcodec_parameters_copy()` 复制编解码器参数
5. **转换时间戳**：使用 `av_rescale_q_rnd()` 转换时间戳
6. **写入数据包**：使用 `av_interleaved_write_frame()` 写入数据包

### 代码结构

- [`TSConverter`](ts_to_mp4.cpp:10) 类：封装转换逻辑
- [`convert()`](ts_to_mp4.cpp:15) 方法：执行转换操作
- [`cleanup()`](ts_to_mp4.cpp:95) 方法：清理资源

## 注意事项

1. 确保FFmpeg库路径正确配置
2. 输入文件必须是有效的TS格式文件
3. 输出文件扩展名应为 `.mp4`
4. 确保有足够的磁盘空间存储输出文件
5. 转换过程中会显示详细的流信息

## 故障排除

### 编译错误

如果遇到编译错误，请检查：
1. FFmpeg库路径是否正确
2. CMake版本是否符合要求
3. 编译器是否支持C++11

### 运行时错误

如果遇到运行时错误，请检查：
1. 输入文件是否存在且可读
2. 输出路径是否有写入权限
3. FFmpeg DLL文件是否在系统PATH中（Windows）

## 许可证

本项目仅供学习和参考使用。

## 作者

MonkeyCode