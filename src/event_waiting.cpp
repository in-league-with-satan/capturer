#include <QDebug>
#include <QThread>

#include "event_waiting.h"

const std::chrono::milliseconds wait_timeout(200);

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

    // condition.wait(ul);
    condition.wait_for(ul, wait_timeout);

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
