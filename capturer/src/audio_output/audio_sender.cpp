/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QCborValue>
#include <QNetworkDatagram>
#include <QDateTime>
#include <qcoreapplication.h>

#include "ff_audio_converter.h"
#include "audio_packet.h"

#include "audio_sender.h"

AudioSender::AudioSender(int dev_num, QObject *parent)
    : QThread(parent)
    , dev_num(dev_num)
{
    frame_buffer=FrameBuffer<Frame::ptr>::make();
    frame_buffer->setMaxSize(1);

    audio_converter=new AudioConverter();

    start(QThread::NormalPriority);
}

AudioSender::~AudioSender()
{
    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }

    delete audio_converter;
}

FrameBuffer<Frame::ptr>::ptr AudioSender::frameBuffer()
{
    return frame_buffer;
}

void AudioSender::setSimplify(bool value)
{
    simplify=value;
}

void AudioSender::run()
{
    QUdpSocket socket;

    socket.moveToThread(this);

    if(!socket.bind(QHostAddress::Any, 4142 + dev_num)) {
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

    const int max_raw_size=65000;

    int max_packets;
    int max_size;

    QList <Listener> listener;

    AudioPacket packet;

    Frame::ptr frame;

    QByteArray ba_in;
    QByteArray ba_out;


    auto sendData=[&socket, &listener](QByteArray &ba) {
        foreach(const Listener &l, listener) {
            socket.writeDatagram(ba, l.host, l.port);

            if(QDateTime::currentMSecsSinceEpoch() - l.timestamp>2000)
                listener.removeAll(l);
        }
    };


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

        frame=frame_buffer->take();

        if(frame) {
            if(listener.isEmpty()) {
                frame.reset();
                continue;
            }

            if(!frame->audio.data_ptr || !frame->audio.data_size || !frame->audio.channels || !frame->audio.sample_size)
                continue;


            if(simplify) {
                if(!audio_converter->compareParams(frame->audio.channels, frame->audio.sample_size==16 ? 2 : 4, 48000, 2, 2, 48000)) {
                    audio_converter->init(av_get_default_channel_layout(frame->audio.channels), 48000, frame->audio.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32,
                                          AV_CH_LAYOUT_STEREO, 48000, AV_SAMPLE_FMT_S16);
                }

                if(audio_converter->isReady()) {
                    audio_converter->convert(frame->audio.data_ptr, frame->audio.data_size, &ba_in);
                    packet.sample_size=16;
                    packet.channels=2;
                }

            } else {
                ba_in=QByteArray((char*)frame->audio.data_ptr, frame->audio.data_size);

                packet.sample_size=frame->audio.sample_size;
                packet.channels=frame->audio.channels;
            }


            if(ba_in.size()>max_raw_size) {
                max_packets=(max_raw_size - (max_raw_size%(packet.channels*packet.sample_size)))/(packet.channels*packet.sample_size);
                max_size=max_packets*packet.channels*packet.sample_size;

                while(!ba_in.isEmpty()) {
                    packet.data=ba_in.left(max_size);

                    ba_out=QCborValue::fromVariant(packet.toExt()).toCbor();

                    sendData(ba_out);

                    ba_in.remove(0, max_size);
                }

            } else {
                packet.data=QByteArray((char*)ba_in.constData(), ba_in.size());

                ba_out=QCborValue::fromVariant(packet.toExt()).toCbor();

                sendData(ba_out);
            }

            frame.reset();
        }
    }
}
