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
#include <QJsonDocument>
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
        bool simplify_audio;

        bool operator==(const Listener &other) {
            return host==other.host && port==other.port;
        }
    };

    const int max_raw_size=65000;

    QList <Listener> listener;

    Frame::ptr frame;

    QByteArray ba_direct;
    QByteArray ba_simple;

    auto sendData=[&socket, &listener](QByteArray ba_direct, QByteArray &ba_simple, int &sample_size, int &channels) {
        QList <QByteArray> ba_cbor_direct;
        QByteArray ba_cbor_simple;

        foreach(const Listener &l, listener) {
            if(l.simplify_audio && !ba_simple.isEmpty()) {
                if(ba_cbor_simple.isEmpty()) {
                    AudioPacket packet;
                    packet.data=ba_simple;
                    packet.channels=2;
                    packet.sample_size=16;
                    ba_cbor_simple=QCborValue::fromVariant(packet.toExt()).toCbor();
                }

                socket.writeDatagram(ba_cbor_simple, l.host, l.port);

            } else {
                if(ba_cbor_direct.isEmpty()) {
                    AudioPacket packet;
                    packet.sample_size=sample_size;
                    packet.channels=channels;

                    const int max_packets=(max_raw_size - (max_raw_size%(packet.channels*packet.sample_size)))/(packet.channels*packet.sample_size);
                    const int max_size=max_packets*packet.channels*packet.sample_size;

                    while(!ba_direct.isEmpty()) {
                        packet.data=ba_direct.left(max_size);
                        ba_direct.remove(0, max_size);

                        ba_cbor_direct.append(QCborValue::fromVariant(packet.toExt()).toCbor());
                    }
                }

                foreach(QByteArray ba, ba_cbor_direct) {
                    socket.writeDatagram(ba, l.host, l.port);
                }
            }


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

            const bool simplify_audio=QJsonDocument::fromJson(dg.data()).toVariant().toMap().value("simplify_audio", false).toBool();

            Listener l={ dg.senderAddress(), dg.senderPort(), QDateTime::currentMSecsSinceEpoch(), simplify_audio };

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


            if(!audio_converter->compareParams(frame->audio.channels, frame->audio.sample_size==16 ? 2 : 4, 48000, 2, 2, 48000)) {
                audio_converter->init(av_get_default_channel_layout(frame->audio.channels), 48000, frame->audio.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32,
                                      AV_CH_LAYOUT_STEREO, 48000, AV_SAMPLE_FMT_S16);
            }

            if(audio_converter->isReady()) {
                audio_converter->convert(frame->audio.data_ptr, frame->audio.data_size, &ba_simple);
            }

            ba_direct=QByteArray((char*)frame->audio.data_ptr, frame->audio.data_size);


            sendData(ba_direct, ba_simple, frame->audio.sample_size, frame->audio.channels);

            frame.reset();
        }
    }
}
