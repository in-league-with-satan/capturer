#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariantMap>
#include <QMutex>

#define settings Settings::instance()

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings *createInstance(QObject *parent=0);
    static Settings *instance();

    bool load();
    bool save();

    QString host();
    void setHost(const QString &value);

    quint16 port();
    void setPort(const quint16 &value);

    QString routingKey();
    void setRoutingKey(const QString &value);

private:
    struct Main {
        QString host;
        quint16 port;
        QString routing_key;

    } main;

    Settings(QObject *parent=0);

    static Settings *_instance;

    QByteArray ba_hash_file;

    QMutex mutex;
};

#endif // SETTINGS_H
