#include <iostream>
#include <string>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class H264ToH265Converter {
private:
    AVFormatContext* inputContext = nullptr;
    AVFormatContext* outputContext = nullptr;
    AVCodecContext* decoderContext = nullptr;
    AVCodecContext* encoderContext = nullptr;
    SwsContext* swsContext = nullptr;
    int videoStreamIndex = -1;

public:
    H264ToH265Converter() = default;
    ~H264ToH265Converter() {
        cleanup();
    }

    bool convert(const std::string& inputFile, const std::string& outputFile) {
        // 打开输入文件
        if (avformat_open_input(&inputContext, inputFile.c_str(), nullptr, nullptr) != 0) {
            std::cerr << "无法打开输入文件: " << inputFile << std::endl;
            return false;
        }

        // 获取流信息
        if (avformat_find_stream_info(inputContext, nullptr) < 0) {
            std::cerr << "无法获取流信息" << std::endl;
            return false;
        }

        // 打印输入信息
        av_dump_format(inputContext, 0, inputFile.c_str(), 0);

        // 查找视频流
        videoStreamIndex = -1;
        for (unsigned int i = 0; i < inputContext->nb_streams; i++) {
            if (inputContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1) {
            std::cerr << "未找到视频流" << std::endl;
            return false;
        }

        // 初始化解码器
        if (!initDecoder()) {
            return false;
        }

        // 创建输出格式上下文
        if (avformat_alloc_output_context2(&outputContext, nullptr, nullptr, outputFile.c_str()) < 0) {
            std::cerr << "无法创建输出格式上下文" << std::endl;
            return false;
        }

        // 初始化编码器
        if (!initEncoder()) {
            return false;
        }

        // 添加视频流到输出
        AVStream* outStream = avformat_new_stream(outputContext, nullptr);
        if (!outStream) {
            std::cerr << "无法创建输出流" << std::endl;
            return false;
        }

        // 复制编码器参数到输出流
        if (avcodec_parameters_from_context(outStream->codecpar, encoderContext) < 0) {
            std::cerr << "无法复制编码器参数" << std::endl;
            return false;
        }

        outStream->time_base = encoderContext->time_base;
        outStream->codecpar->codec_tag = 0;

        // 打印输出信息
        av_dump_format(outputContext, 0, outputFile.c_str(), 1);

        // 打开输出文件
        if (!(outputContext->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&outputContext->pb, outputFile.c_str(), AVIO_FLAG_WRITE) < 0) {
                std::cerr << "无法打开输出文件: " << outputFile << std::endl;
                return false;
            }
        }

        // 写入文件头
        if (avformat_write_header(outputContext, nullptr) < 0) {
            std::cout << "can not write header file" <<std::endl;
            return false;
        }

        // 转码循环
        if (!transcode()) {
            return false;
        }

        // 写入文件尾
        av_write_trailer(outputContext);
        std::cout <<  "conver success, output file"<<std::endl;
        return true;
    }

private:
    bool initDecoder() {
        AVCodecParameters* codecPar = inputContext->streams[videoStreamIndex]->codecpar;
        
        // 查找H.264解码器
        const AVCodec* decoder = avcodec_find_decoder(codecPar->codec_id);
        if (!decoder) {
            std::cerr << "未找到解码器" << std::endl;
            return false;
        }

        // 创建解码器上下文
        decoderContext = avcodec_alloc_context3(decoder);
        if (!decoderContext) {
            std::cerr << "无法创建解码器上下文" << std::endl;
            return false;
        }

        // 复制编解码器参数
        if (avcodec_parameters_to_context(decoderContext, codecPar) < 0) {
            std::cerr << "无法复制编解码器参数" << std::endl;
            return false;
        }

        // 打开解码器
        if (avcodec_open2(decoderContext, decoder, nullptr) < 0) {
            std::cerr << "无法打开解码器" << std::endl;
            return false;
        }

        std::cout << "解码器初始化成功: " << decoder->name << std::endl;
        return true;
    }

    bool initEncoder() {
        // 查找H.265编码器
        const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_HEVC);
        if (!encoder) {
            std::cerr << "未找到H.265编码器" << std::endl;
            return false;
        }

        // 创建编码器上下文
        encoderContext = avcodec_alloc_context3(encoder);
        if (!encoderContext) {
            std::cerr << "无法创建编码器上下文" << std::endl;
            return false;
        }

        // 设置编码器参数
        encoderContext->width = decoderContext->width;
        encoderContext->height = decoderContext->height;
        encoderContext->time_base = AVRational{1, 30};  // 30 FPS
        encoderContext->framerate = AVRational{30, 1};
        encoderContext->gop_size = 10;
        encoderContext->max_b_frames = 1;
        encoderContext->pix_fmt = AV_PIX_FMT_YUV420P;

        // 设置编码质量
        if (encoder->id == AV_CODEC_ID_H264 || encoder->id == AV_CODEC_ID_HEVC) {
            av_opt_set(encoderContext->priv_data, "preset", "medium", 0);
            av_opt_set(encoderContext->priv_data, "crf", "28", 0);
        }

        // 打开编码器
        if (avcodec_open2(encoderContext, encoder, nullptr) < 0) {
            std::cerr << "无法打开编码器" << std::endl;
            return false;
        }

        std::cout << "编码器初始化成功: " << encoder->name << std::endl;
        std::cout << "分辨率: " << encoderContext->width << "x" << encoderContext->height << std::endl;
        return true;
    }

    bool transcode() {
        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        AVFrame* outputFrame = av_frame_alloc();
        
        if (!packet || !frame || !outputFrame) {
            std::cerr << "无法分配内存" << std::endl;
            return false;
        }

        // 初始化输出帧
        outputFrame->format = encoderContext->pix_fmt;
        outputFrame->width = encoderContext->width;
        outputFrame->height = encoderContext->height;
        if (av_frame_get_buffer(outputFrame, 0) < 0) {
            std::cerr << "无法分配输出帧缓冲区" << std::endl;
            return false;
        }

        int frameCount = 0;
        int64_t pts = 0;

        while (av_read_frame(inputContext, packet) >= 0) {
            if (packet->stream_index == videoStreamIndex) {
                // 发送数据包到解码器
                int ret = avcodec_send_packet(decoderContext, packet);
                if (ret < 0) {
                    std::cerr << "发送数据包到解码器失败" << std::endl;
                    av_packet_unref(packet);
                    continue;
                }

                // 接收解码后的帧
                while (ret >= 0) {
                    ret = avcodec_receive_frame(decoderContext, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        std::cerr << "解码错误" << std::endl;
                        break;
                    }

                    // 转换像素格式（如果需要）
                    if (frame->format != encoderContext->pix_fmt) {
                        if (!swsContext) {
                            swsContext = sws_getContext(
                                frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
                                encoderContext->width, encoderContext->height, encoderContext->pix_fmt,
                                SWS_BILINEAR, nullptr, nullptr, nullptr
                            );
                            if (!swsContext) {
                                std::cerr << "无法初始化SWS上下文" << std::endl;
                                return false;
                            }
                        }

                        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height,
                                  outputFrame->data, outputFrame->linesize);
                    } else {
                        // 直接复制数据
                        av_frame_copy(outputFrame, frame);
                    }

                    outputFrame->pts = pts++;

                    // 发送帧到编码器
                    ret = avcodec_send_frame(encoderContext, outputFrame);
                    if (ret < 0) {
                        std::cerr << "发送帧到编码器失败" << std::endl;
                        break;
                    }

                    // 接收编码后的数据包
                    while (ret >= 0) {
                        ret = avcodec_receive_packet(encoderContext, packet);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        } else if (ret < 0) {
                            std::cerr << "编码错误" << std::endl;
                            break;
                        }

                        // 转换时间戳
                        AVStream* outStream = outputContext->streams[0];
                        packet->stream_index = outStream->index;
                        av_packet_rescale_ts(packet, encoderContext->time_base, outStream->time_base);
                        packet->pos = -1;

                        // 写入数据包
                        if (av_interleaved_write_frame(outputContext, packet) < 0) {
                            std::cerr << "写入数据包失败" << std::endl;
                            av_packet_unref(packet);
                            break;
                        }

                        av_packet_unref(packet);
                    }

                    frameCount++;
                    if (frameCount % 30 == 0) {
                        std::cout << "已处理 " << frameCount << " 帧..." << std::endl;
                    }
                }
            }
            av_packet_unref(packet);
        }

        // 刷新编码器
        avcodec_send_frame(encoderContext, nullptr);
        while (avcodec_receive_packet(encoderContext, packet) >= 0) {
            AVStream* outStream = outputContext->streams[0];
            packet->stream_index = outStream->index;
            av_packet_rescale_ts(packet, encoderContext->time_base, outStream->time_base);
            packet->pos = -1;

            if (av_interleaved_write_frame(outputContext, packet) < 0) {
                std::cerr << "写入数据包失败" << std::endl;
            }
            av_packet_unref(packet);
        }

        std::cout << "总共处理 " << frameCount << " 帧" << std::endl;

        av_packet_free(&packet);
        av_frame_free(&frame);
        av_frame_free(&outputFrame);

        return true;
    }

    void cleanup() {
        if (swsContext) {
            sws_freeContext(swsContext);
            swsContext = nullptr;
        }
        if (encoderContext) {
            avcodec_free_context(&encoderContext);
        }
        if (decoderContext) {
            avcodec_free_context(&decoderContext);
        }
        if (outputContext && !(outputContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&outputContext->pb);
        }
        if (outputContext) {
            avformat_free_context(outputContext);
            outputContext = nullptr;
        }
        if (inputContext) {
            avformat_close_input(&inputContext);
            inputContext = nullptr;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "用法: " << argv[0] << " <输入.h264文件> <输出.h265文件>" << std::endl;
        std::cout << "示例: " << argv[0] << " input.h264 output.h265" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::cout << "开始转码..." << std::endl;
    std::cout << "输入文件: " << inputFile << std::endl;
    std::cout << "输出文件: " << outputFile << std::endl;

    H264ToH265Converter converter;
    if (converter.convert(inputFile, outputFile)) {
        std::cout << "转码完成！" << std::endl;
        return 0;
    } else {
        std::cerr << "转码失败！" << std::endl;
        return 1;
    }
}