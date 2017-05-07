#ifndef EVENT_WAITING_H
#define EVENT_WAITING_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class EventWaiting : public QObject
{
    Q_OBJECT

public:
    explicit EventWaiting(QObject *parent=0);
    ~EventWaiting();

    void wait();
    void next();

private:
    QMutex mutex;
    // QWaitCondition wait_condition;
};

#endif // EVENT_WAITING_H
