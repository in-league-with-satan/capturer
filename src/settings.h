#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariantMap>

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
        int rgb_10bit;
        int restart;

    } device_decklink;


    struct DeviceQCam {
        int index;
        int resolution;
        int framerate;
        int pixel_format;
        int restart;

    } device_cam;

    struct Rec {
        QVariantMap pixel_format;
        QVariantMap preset;
        int pixel_format_current;
        int preset_current;
        int crf;
        int encoder;
        int half_fps;
        int stop_rec_on_frames_drop;
        int downscale;
        int scale_filter;

    } rec;

    struct HttpServer {
        quint16 port;
        bool enabled;

    } http_server;

    struct Keys {
        QMap <int, int> code; // Qt::Key : KeyCodeC
        bool need_setup;

    } keyboard_shortcuts;

private:
    Settings(QObject *parent=0);

    static Settings *_instance;

    QByteArray ba_hash_file;
};

#endif // SETTINGS_H
