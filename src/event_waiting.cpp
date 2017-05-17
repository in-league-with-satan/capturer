#include <QDebug>
#include <QThread>

#include "event_waiting.h"

EventWaiting::EventWaiting(QObject *parent)
    : QObject(parent)
{
    mutex.lock();
}

EventWaiting::~EventWaiting()
{
    mutex.tryLock();

    mutex.unlock();
}

void EventWaiting::wait()
{
    mutex.lock();

    // wait_condition.wait(&mutex);

    // mutex.unlock();
}

void EventWaiting::next()
{
    mutex.tryLock();

    mutex.unlock();

    // wait_condition.wakeAll();
}
