#ifndef FF_MEDIA_INFO_THREAD_H
#define FF_MEDIA_INFO_THREAD_H

#include <QThread>
#include <QMutex>
#include <QQueue>



class QTimer;

class FFMediaInfoThread : public QThread
{
    Q_OBJECT

public:
    FFMediaInfoThread(QObject *parent=0);

public slots:
    void pause(bool state);
    void enqueue(const QString &filename);

private slots:
    void checkQueue();

protected:
    void run();

private:
    QTimer *timer;

    QQueue <QString> queue;
    QMutex mutex_queue;

    std::atomic <bool> on_pause;

signals:
    void ready(QString key, QString info);
};

#endif // FF_MEDIA_INFO_THREAD_H
