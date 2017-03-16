#include "event_waiting.h"

EventWaiting::EventWaiting(QObject *parent)
    : QObject(parent)
{
    mutex.lock();
}

EventWaiting::~EventWaiting()
{
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
