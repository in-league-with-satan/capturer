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
