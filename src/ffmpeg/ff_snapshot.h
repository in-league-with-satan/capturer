#ifndef FF_SNAPSHOT_H
#define FF_SNAPSHOT_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QPixmap>

#include <atomic>

class QTimer;
class Database;

class FFSnapshot : public QThread
{
    Q_OBJECT

public:
    FFSnapshot(QObject *paretn=0);

public slots:
    void enqueue(const QString &filename);

    void pause(bool state);
    void viewerVisible(bool value);

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

    std::atomic <bool> on_pause;
    std::atomic <bool> viewer_visible;

    Database *database;

signals:
    void ready(QString key, QImage image);
};

#endif // FF_SNAPSHOT_H
