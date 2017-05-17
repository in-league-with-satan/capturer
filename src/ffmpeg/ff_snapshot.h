#ifndef FF_SNAPSHOT_H
#define FF_SNAPSHOT_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QPixmap>

class QTimer;

class FFSnapshot : public QThread
{
    Q_OBJECT

public:
    FFSnapshot(QObject *paretn=0);

public slots:
    void enqueue(QString file);

protected:
    void run();

private slots:
    void checkQueue();

private:
    QTimer *timer;

    bool accurate_seek;
    int shots_per_10_min;

    QQueue <QString> queue;
    QMutex mutex_queue;

signals:
    void ready(QString key, QImage image);

};

#endif // FF_SNAPSHOT_H
