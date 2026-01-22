#include <iostream>
#include <string>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

class TSConverter {
private:
    AVFormatContext* inputContext = nullptr;
    AVFormatContext* outputContext = nullptr;

public:
    TSConverter() = default;
    ~TSConverter() {
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

        // 创建输出格式上下文
        if (avformat_alloc_output_context2(&outputContext, nullptr, nullptr, outputFile.c_str()) < 0) {
            std::cerr << "无法创建输出格式上下文" << std::endl;
            return false;
        }

        // 复制流
        for (unsigned int i = 0; i < inputContext->nb_streams; i++) {
            AVStream* inStream = inputContext->streams[i];
            AVStream* outStream = avformat_new_stream(outputContext, nullptr);
            
            if (!outStream) {
                std::cerr << "无法创建输出流" << std::endl;
                return false;
            }

            // 复制流参数
            if (avcodec_parameters_copy(outStream->codecpar, inStream->codecpar) < 0) {
                std::cerr << "无法复制流参数" << std::endl;
                return false;
            }

            outStream->codecpar->codec_tag = 0;
        }

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
            std::cerr << "无法写入文件头" << std::endl;
            return false;
        }

        // 转换并写入数据包
        AVPacket packet;
        av_init_packet(&packet);
        packet.data = nullptr;
        packet.size = 0;

        while (av_read_frame(inputContext, &packet) >= 0) {
            AVStream* inStream = inputContext->streams[packet.stream_index];
            AVStream* outStream = outputContext->streams[packet.stream_index];

            // 转换时间戳
            packet.pts = av_rescale_q_rnd(packet.pts, inStream->time_base, outStream->time_base,
                                          static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            packet.dts = av_rescale_q_rnd(packet.dts, inStream->time_base, outStream->time_base,
                                          static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
            packet.pos = -1;

            // 写入数据包
            if (av_interleaved_write_frame(outputContext, &packet) < 0) {
                std::cerr << "写入数据包失败" << std::endl;
                av_packet_unref(&packet);
                return false;
            }

            av_packet_unref(&packet);
        }

        // 写入文件尾
        av_write_trailer(outputContext);

        std::cout << "转换成功！输出文件: " << outputFile << std::endl;
        return true;
    }

private:
    void cleanup() {
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
        std::cout << "用法: " << argv[0] << " <输入.ts文件> <输出.mp4文件>" << std::endl;
        std::cout << "示例: " << argv[0] << " input.ts output.mp4" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::cout << "开始转换..." << std::endl;
    std::cout << "输入文件: " << inputFile << std::endl;
    std::cout << "输出文件: " << outputFile << std::endl;

    TSConverter converter;
    if (converter.convert(inputFile, outputFile)) {
        std::cout << "转换完成！" << std::endl;
        return 0;
    } else {
        std::cerr << "转换失败！" << std::endl;
        return 1;
    }
}