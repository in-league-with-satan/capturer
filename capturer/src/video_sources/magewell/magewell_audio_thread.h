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

#ifndef MAGEWELL_AUDIO_THREAD_H
#define MAGEWELL_AUDIO_THREAD_H

#include <QThread>
#include <QMutex>
#include <QSize>

#include <atomic>

#include "magewell_global.h"
#include "source_interface.h"
#include "frame_buffer.h"


class MagewellAudioContext;

class MagewellAudioThread : public QThread
{
    Q_OBJECT

public:
    explicit MagewellAudioThread(QObject *parent=0);
    virtual ~MagewellAudioThread();

    int channels() const;
    int sampleSize() const;

    QByteArray getData();

public slots:
    void deviceStop();
    void setChannel(MGHCHANNEL channel);

protected:
    void run();

private:
    void deviceStart();
    void updateAudioSignalInfo();

    std::atomic <MGHCHANNEL> current_channel;
    std::atomic <bool> running;

    MagewellAudioContext *d;

    QByteArray ba_data;

    QMutex mutex;

signals:
    void audioSampleSizeChnanged(SourceInterface::AudioSampleSize::T value);
};

#endif // MAGEWELL_AUDIO_THREAD_H
