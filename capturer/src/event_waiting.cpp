/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QThread>

#include "event_waiting.h"

const std::chrono::milliseconds wait_timeout(1000);

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
