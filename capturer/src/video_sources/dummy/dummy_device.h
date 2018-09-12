/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef DUMMY_DEVICE_H
#define DUMMY_DEVICE_H

#include <QThread>

#include <atomic>

#include "source_interface.h"
#include "glazing_ribbon.h"

class DummyDevice : public QThread, public SourceInterface
{
    Q_OBJECT
    Q_INTERFACES(SourceInterface)

public:
    explicit DummyDevice(QObject *parent=0);
    ~DummyDevice();

    Type::T type() const;

    bool isActive();
    bool gotSignal();

    struct Device {
        QSize frame_size;
        bool show_frame_counter;
    };

    typedef QList<QSize> Framesizes;

    static Framesizes availableFramesizes();

protected:
    void run();

public slots:
    void deviceStart();
    void deviceStop();

    void setDevice(void *ptr);

private:
    std::atomic <bool> running;
    std::atomic <bool> running_thread;

    GlazingRibbon glazing_ribbon;

    qint64 time_last_frame=0;

    bool frame_counter;

    QMutex mutex;

signals:
    void signalLost(bool value);
    void formatChanged(QString format);
    void frameSkipped();
    void errorString(QString err_string);
};

#endif // DUMMY_DEVICE_H
