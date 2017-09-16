#include <QDebug>
#include <QThread>

#include "event_waiting.h"

EventWaiting::EventWaiting(QObject *parent)
    : QObject(parent)
{
    // mutex.lock();
}

EventWaiting::~EventWaiting()
{
    next();
}

void EventWaiting::wait()
{
    // mutex.lock();

    //

    // wait_condition.wait(&mutex);

    // mutex.unlock();

    //

    std::unique_lock <std::mutex> ul(mutex);

    condition.wait(ul);

    ul.unlock();
}

void EventWaiting::next()
{
    // mutex.tryLock();

    // mutex.unlock();

    //

    // wait_condition.wakeAll();

    //

    condition.notify_all();
}
