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

    if(!socket.bind(QHostAddress::LocalHost, 4142)) {
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
