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

    } main;

    struct Device {
        int index;
        int audio_sample_size;
        int restart;

    } device;

    struct Rec {
        int pixel_format_current;
        QVariantMap pixel_format;
        int crf;
        int encoder;
        int half_fps;
        int stop_rec_on_frames_drop;

    } rec;

    struct HttpServer {
        quint16 port;
        bool enabled;

    } http_server;

private:
    Settings(QObject *parent=0);

    static Settings *_instance;

    QByteArray ba_hash_file;
};

#endif // SETTINGS_H
