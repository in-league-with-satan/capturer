#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include <QFile>
#include <QImage>

struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct SwsContext;
struct AVPacket;

class FFMpeg : public QObject
{
    Q_OBJECT

public:
    FFMpeg(QObject *parent=0);
    ~FFMpeg();

    bool initVideoCoder(QSize size);

    bool appendFrame(QImage video_frame, QByteArray ba_audio);

    bool stopCoder();


private:
    QFile f_out;

    AVCodec *av_codec;
    AVCodecContext *av_codec_context;
    SwsContext *convert_context;

    AVFrame *av_frame;
    AVFrame *av_frame_converted;

    AVPacket *av_packet;

    size_t frame_num;

    bool processing;


};

#endif // FFMPEG_H
