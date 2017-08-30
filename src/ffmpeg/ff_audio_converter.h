#ifndef FF_AUDIO_CONVERTER_H
#define FF_AUDIO_CONVERTER_H

#include <QByteArray>

class SwrContext;

class AudioConverter
{
public:
    AudioConverter();
    ~AudioConverter();

    bool isReady() const;

    bool init(uint64_t in_channels_layout, int64_t in_sample_rate, int64_t in_sample_format,
              uint64_t out_channels_layout, int64_t out_sample_rate, int64_t out_sample_format);

    bool convert(void *src, size_t size, QByteArray *dst);
    bool convert(QByteArray *src, QByteArray *dst);

    void free();

    uint64_t inChannelsLayout() const;
    uint64_t inChannels() const;
    int64_t inSampleRate() const;
    int64_t inSampleFormat() const;

    uint64_t outChannelsLayout() const;
    uint64_t outChannels() const;
    int64_t outSampleRate() const;
    int64_t outSampleFormat() const;

private:
    SwrContext *context;

    uint64_t in_channels_layout;
    uint64_t in_channels;
    size_t in_sample_size;
    int64_t in_sample_rate;
    int64_t in_sample_format;

    uint64_t out_channels_layout;
    uint64_t out_channels;
    size_t out_sample_size;
    int64_t out_sample_rate;
    int64_t out_sample_format;
};

#endif // FF_AUDIO_CONVERTER_H
