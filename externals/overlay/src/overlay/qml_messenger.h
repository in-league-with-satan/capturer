#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>

class QmlMessenger : public QObject
{
    Q_OBJECT

public:
    explicit QmlMessenger(QObject *parent=0);

    ~QmlMessenger();

    void recStats(QString duration, QString bitrate, QString size);

public slots:

signals:
    void updateRecStats(QString duration, QString bitrate, QString size);

    void recStarted();
    void recStopped();

};

#endif // QML_MESSENGER_H
