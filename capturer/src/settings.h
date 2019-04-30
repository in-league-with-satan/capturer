/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariantMap>

#include "ff_encoder.h"

#define settings Settings::instance()

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings *createInstance(QObject *parent=0);
    static Settings *instance();

    bool load();
    bool save();

    struct Main {
        int preview=1;
        int dummy;
        int headless=0;
        int simplify_audio_for_send=0;
        int source_device_add=0;
        int source_device_remove=0;
        QString location_videos;
        QVariantMap supported_enc;

    } main;

    struct SourceDevice {
        enum {
            MaxNum=10
        };

        QString group;
        QString group_settings;
        int dummy;
        int index=0;
        int start;
        int stop;

        struct DummyDevice {
            int framesize=0;
            int show_frame_counter=0;

        } dummy_device;

        struct FFDevice {
            int index_video=0;
            int index_audio=0;
            int framesize=0;
            int framerate=0;
            int pixel_format=0;

        } ff_device;

        struct Magewell {
            int index=0;
            int pixel_format=0;
            int framesize=0;
            int color_format_in=0;
            int color_format_out=0;
            int quantization_range_in=0;
            int quantization_range_out=0;
            int audio_remap_mode=0;
            int low_latency=0;
            int half_fps=0;
            int pts_mode=0;

        } magewell;

        struct Decklink {
            int index=0;
            int audio_sample_size=0;
            int video_bitdepth=0;

        } decklink;

        struct Rec {
            int pixel_format_current;
            int preset_current;
            int crf;
            int encoder_audio;
            int encoder_video;
            int half_fps;
            int direct_stream_copy;
            int downscale;
            int scale_filter;
            int check_encoders;
            int color_primaries;
            int color_space;
            int color_transfer_characteristic;
            int sws_color_space_src;
            int sws_color_space_dst;
            int sws_color_range_src;
            int sws_color_range_dst;
            FFEncoder::Config::NVEnc nvenc;
            QVariantMap pixel_format;
            QVariantMap preset;

        } rec;
    };

    SourceDevice *sourceDevice(uint8_t index);
    SourceDevice *sourceDeviceAdd();
    void sourceDeviceRemove();

    void renumSourceDevices();

    QList <SourceDevice> source_device;
    QVariantMap getSourceDeviceSettings(const SourceDevice &device);
    void setSourceDeviceSettings(SourceDevice *device, const QVariantMap &map_root);

    struct HttpServer {
        quint16 port;
        bool enabled;

    } http_server;

    struct Keys {
        QMap <int, int> code; // Qt::Key : KeyCodeC
        bool need_setup;

    } keyboard_shortcuts;

    void checkEncoders();

private:
    Settings(QObject *parent=0);

    static Settings *_instance;

    QByteArray ba_hash_file;
};

#endif // SETTINGS_H
