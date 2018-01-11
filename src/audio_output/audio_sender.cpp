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

#include <QDebug>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QNetworkDatagram>
#include <QDateTime>
#include <qcoreapplication.h>

#include "audio_packet.h"

#include "audio_sender.h"

AudioSender::AudioSender(QObject *parent)
    : QThread(parent)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    start(QThread::NormalPriority);
}

AudioSender::~AudioSender()
{
    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer::ptr AudioSender::frameBuffer()
{
    return frame_buffer;
}

void AudioSender::run()
{
    QUdpSocket socket;

    socket.moveToThread(this);

    if(!socket.bind(QHostAddress::Any, 4142)) {
        qCritical() << socket.errorString();
        return;
    }

    struct Listener {
        QHostAddress host;
        int port;
        qint64 timestamp;

        bool operator==(const Listener &other) {
            return host==other.host && port==other.port;
        }
    };

    QList <Listener> listener;

    AudioPacket packet;

    Frame::ptr frame;

    running=true;

    while(running) {
        while(socket.hasPendingDatagrams()) {
            QNetworkDatagram dg=socket.receiveDatagram();

            if(!dg.isValid())
                continue;

            Listener l={ dg.senderAddress(), dg.senderPort(), QDateTime::currentMSecsSinceEpoch() };

            listener.removeAll(l);
            listener.append(l);
        }

        frame_buffer->wait();

        if(listener.isEmpty()) {
            continue;
        }

        frame=frame_buffer->take();

        if(frame) {
            packet.data=QByteArray(frame->audio.ptr_data, frame->audio.data_size);
            packet.channels=frame->audio.channels;
            packet.sample_size=frame->audio.sample_size;

            QByteArray ba=QJsonDocument::fromVariant(packet.toExt()).toBinaryData();

            foreach(const Listener &l, listener) {
                socket.writeDatagram(ba, l.host, l.port);

                if(QDateTime::currentMSecsSinceEpoch() - l.timestamp>2000)
                    listener.removeAll(l);
            }

            frame.reset();
        }
    }
}
