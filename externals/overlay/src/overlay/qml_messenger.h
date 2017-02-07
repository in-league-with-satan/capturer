#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>

class QmlMessenger : public QObject
{
    Q_OBJECT

public:
    explicit QmlMessenger(QObject *parent=0);

    ~QmlMessenger();

public slots:
    void hello();

signals:
    void updateRecStats(QString duration, QString bitrate, QString size);

};

#endif // QML_MESSENGER_H
