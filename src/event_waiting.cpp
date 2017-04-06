#include "event_waiting.h"

EventWaiting::EventWaiting(QObject *parent)
    : QObject(parent)
{
    mutex.lock();
}

EventWaiting::~EventWaiting()
{
    while(!mutex.tryLock(10))
        mutex.unlock();

    mutex.unlock();
}

void EventWaiting::wait()
{
    mutex.lock();
}

void EventWaiting::next()
{
    mutex.tryLock();

    mutex.unlock();
}
