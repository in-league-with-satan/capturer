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
    mutex.unlock();

    // wait_condition.wakeAll();
}

void EventWaiting::wait()
{
    mutex.lock();

    // wait_condition.wait(&mutex);

    // mutex.unlock();
}

void EventWaiting::next()
{
    mutex.unlock();

    // wait_condition.wakeAll();
}
