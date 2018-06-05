/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
        int preview;
        int smooth_transform;
        int dummy;

    } main;

    struct DeviceDecklink {
        int index;
        int audio_sample_size;
        int half_fps;
        int video_depth_10bit;
        int restart;

        struct {
            bool enabled=false;
            bool frame_counter=false;
            int frame_height=1080;

        } dummy;

    } device_decklink;


    struct DeviceCam {
        int index_video;
        int index_audio;
        int resolution;
        int framerate;
        int pixel_format;
        int restart;
        int stop;

    } device_cam;

    struct Rec {
        QVariantMap pixel_format;
        QVariantMap preset;
        QVariantMap supported_enc;
        int pixel_format_current;
        int preset_current;
        int crf;
        int encoder_audio;
        int encoder_video;
        int half_fps;
        int stop_rec_on_frames_drop;
        int downscale;
        int scale_filter;
        int check_encoders;
        int color_primaries;
        int color_space;
        int color_transfer_characteristic;

    } rec;

    FFEncoder::Config::NVEnc nvenc;

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
