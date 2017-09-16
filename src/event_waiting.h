#ifndef EVENT_WAITING_H
#define EVENT_WAITING_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

#include <mutex>
#include <condition_variable>

class EventWaiting : public QObject
{
    Q_OBJECT

public:
    explicit EventWaiting(QObject *parent=0);
    ~EventWaiting();

    void wait();
    void next();

private:
    // QMutex mutex;
    // QWaitCondition wait_condition;

    std::mutex mutex;
    std::condition_variable condition;
};

#endif // EVENT_WAITING_H
