#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

#include <SDL.h>
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug() << av_version_info();

    SDL_version version;
    SDL_VERSION(&version);
    qDebug()<<"sdl version major is"<<version.major;
    qDebug()<<"sdl version minor is"<<version.minor;


    testVideoExtractAAC();


}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::videoExtractAAC(const char* videoFile,const char* aacFile)
{
    AVFormatContext* formatContext = nullptr;
    int audioStreamIndex = -1;
    AVPacket packet;
    FILE* outputFile = nullptr;

    // 打开输入文件
    if (avformat_open_input(&formatContext, videoFile, nullptr, nullptr) < 0) {
        qCritical() << "Error: Could not open input file " << videoFile;
        return false;
    }

    // 获取文件流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        qCritical() << "Error: Could not find stream information";
        avformat_close_input(&formatContext);
        return false;
    }

    // 查找音频流
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        qCritical() << "Error: Could not find audio stream" ;
        avformat_close_input(&formatContext);
        return false;
    }

    if (AVCodecID::AV_CODEC_ID_AAC != formatContext->streams[audioStreamIndex]->codecpar->codec_id){
        qCritical() << "Error: audio codec isn't aac" ;
        avformat_close_input(&formatContext);
        return false;
    }

    // 打开输出文件
    outputFile = fopen(aacFile, "wb");
    if (!outputFile) {
        qCritical() << "Error: Could not open output file " << aacFile ;
        avformat_close_input(&formatContext);
        return false;
    }

    int packetLen = 0;
    // 读取并解码音频流
    while (av_read_frame(formatContext, &packet) >= 0) {
        // 只有音频数据才处理
        if (packet.stream_index == audioStreamIndex) {
            //adts head有7字节
            char adts_header_buffer[7] = {0};
            adts_header(adts_header_buffer,
                        packet.size,
                        formatContext->streams[audioStreamIndex]->codecpar->profile,
                        formatContext->streams[audioStreamIndex]->codecpar->sample_rate,
                        formatContext->streams[audioStreamIndex]->codecpar->channels);
            //写入adts头
            fwrite(adts_header_buffer, 1, 7, outputFile);
            //写入adts体
            packetLen = fwrite(packet.data, 1, packet.size, outputFile);
            if (packetLen != packet.size){
                qCritical() << "length of writed data isn't equal packet.size,packetLen="<<packetLen<<" packet.size="<<packet.size;
            }
        }

        av_packet_unref(&packet);
    }

    // 清理和释放资源
    fclose(outputFile);
    avformat_close_input(&formatContext);

    qDebug() << "Extraction completed successfully!" ;
    return true;
}

#define ADTS_HEADER_LEN 7
const int sampling_frequencies[] = {
    96000,  //0x0
    88200,  //0x1
    64000,  //0x2
    48000,  //0x3
    44100,  //0x4
    32000,  //0x5
    24000,  //0x6
    22050,  //0x7
    16000,  //0x8
    12000,  //0x9
    11025,  //0xa
    8000,   //0xb
    7350    //0xc
    // 0xd e f是保留的
};
int MainWindow::adts_header(char *adts_header, int data_length, int profile, int samplerate, int channels)
{
    //默认使用48000hz
    int sampling_frequency_index = 3;
    int adtsLen = data_length +7;

    int frequenceies_size = sizeof(sampling_frequencies)/sizeof(sampling_frequencies[0]);
    int i = 0;
    for(i = 0; i< frequenceies_size; i++){
        if (samplerate == sampling_frequencies[i]){
            sampling_frequency_index = i;
            break;
        }
    }
    if (i >= frequenceies_size){
        qDebug() << "unsupport samplerate:" << samplerate;
        return -1;
    }

    //前28bits为固定头，后28bit为可变头，定头信息中的数据每一帧都相同，而可变头信息则在帧与帧之间可变
    adts_header[0] = 0xff;          //syncword:0xfff 高8bits
    adts_header[1] = 0xf0;          //syncword:0xfff 低8bits
    adts_header[1] |= (0 << 3);     //MPEG Version:0 for MPEG-4,1 for MPEG-2    1bit
    adts_header[1] |= (0 << 1);     //Layer:0   2bits
    adts_header[1] |= 1;            //protection absent:1

    adts_header[2] = profile << 6;  //profile:profile   2bits
    adts_header[2] |= (sampling_frequency_index & 0x0f) << 2;   //sampling frequency index:sampling_frequency_index 4bits
    adts_header[2] |= (0 << 1);     //private bit:0
    adts_header[2] |= (channels & 0x04) >> 2;   //channel configuration:channels 高1bit

    adts_header[3] = (channels & 0x03) << 6;        //channel configuration:channels 低2bits
    adts_header[3] |= (0 << 5);                     //original:0 1bit
    adts_header[3] |= (0 << 4);                     //home:0 1bit
    adts_header[3] |= (0 << 3);                     //copyright id bit:0 1bit
    adts_header[3] |= (0 << 2);                     //copyright id start:0 1bit
    adts_header[3] |= ((adtsLen & 0x1800) >> 11);   //frame length:value 高2bits

    adts_header[4] = (uint8_t)((adtsLen & 0x7f8) >> 3); //frame length:value 高8bits
    adts_header[5] = (uint8_t)((adtsLen & 0x7) << 5);   //frame length:value 低3bits
    adts_header[5] |= 0x1f;                             //buffer fullness:0x7ff 高8bits
    adts_header[6] = 0xfc;                              //11111100 buffer fullness:0x7ff 低6bits

    return 0;
}

void MainWindow::testVideoExtractAAC()
{
    QString exeDir = QDir::currentPath();
    qDebug() << "exeDir" << exeDir;
    std::string videoFile = exeDir.toStdString() + "/media/test.flv";
    std::string aacFile = exeDir.toStdString() + "/media/test-output.aac";
    videoExtractAAC(videoFile.c_str(),aacFile.c_str());
}
