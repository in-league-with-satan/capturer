#include <QDebug>
#include <QDataStream>
#include <QJsonDocument>

#include "data_types.h"

#include "client_handler.h"


ClientHandler::ClientHandler(qintptr socket_descriptor, QObject *parent)
    : QThread(parent)
    , socket_descriptor(socket_descriptor)
{
    start();
}

void ClientHandler::disconnected()
{
    quit();
}

void ClientHandler::read()
{
    timer->stop();

    write();

    if(socket->bytesAvailable()<8) {
        timer->start();
        return;
    }

    QDataStream stream(socket);

    quint32 marker;
    qint32 data_size;

    stream >> marker;
    stream >> data_size;

    if(marker!=MARKER) {
        qCritical() << "wrong marker" << marker << MARKER;
        socket->abort();
        timer->start();
        return;
    }

    QByteArray ba_data;

    while(ba_data.size()!=data_size) {
        if(socket->bytesAvailable()<1) {
            if(!socket->waitForReadyRead(READ_TIMEOUT)) {
                qCritical() << "timeout";
                socket->abort();
                return;
            }
        }

        ba_data.append(socket->read(data_size - ba_data.size()));
    }

    QVariantMap map=
        QJsonDocument::fromBinaryData(ba_data, QJsonDocument::BypassValidation).toVariant().toMap();

    switch(map.value("cmd", -1).toInt()) {
    case Command::GetProtocolVersion:
        cmdProtocolVersion(&map);
        break;

    case Command::KeyPressed:
        cmdKeyPressed(&map);
        break;

    case Command::PlayerSeek:
        cmdPlayerSeek(&map);
        break;

    default:
        break;
    }

    read();

    timer->start();
}

void ClientHandler::write()
{
    QVariantMap map;

    while(true) {
        {
            QMutexLocker ml(&mutex_queue);

            if(queue_send.isEmpty())
                return;

            map=queue_send.dequeue();
        }

        sendData(map);
    }
}

void ClientHandler::sendRecState(bool value)
{
    QVariantMap vm;

    vm.insert("msg", Message::RecStateChanged);
    vm.insert("data", value);

    QMutexLocker ml(&mutex_queue);

    queue_send.enqueue(vm);
}

void ClientHandler::sendRecStats(NRecStats value)
{
    QVariantMap vm;

    vm.insert("msg", Message::RecStats);
    vm.insert("data", value.toExt());

    QMutexLocker ml(&mutex_queue);

    queue_send.enqueue(vm);
}

void ClientHandler::sendPlayerDuration(qint64 value)
{
    QVariantMap vm;

    vm.insert("msg", Message::PlayerDurationChanged);
    vm.insert("data", value);

    QMutexLocker ml(&mutex_queue);

    queue_send.enqueue(vm);
}

void ClientHandler::sendPlayerPosition(qint64 value)
{
    QVariantMap vm;

    vm.insert("msg", Message::PlayerPositionChanged);
    vm.insert("data", value);

    QMutexLocker ml(&mutex_queue);

    queue_send.enqueue(vm);
}

void ClientHandler::run()
{
    socket=new QTcpSocket();

    socket->moveToThread(this);

    socket->setSocketOption(QAbstractSocket::LowDelayOption, true);
    socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);

    connect(socket, SIGNAL(readyRead()), SLOT(read()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), SLOT(disconnected()), Qt::DirectConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(disconnected()), Qt::DirectConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(disconnected()), Qt::DirectConnection);

    if(!socket->setSocketDescriptor(socket_descriptor)) {
        qCritical() << socket->errorString();
        return;
    }

    timer=new QTimer();
    timer->moveToThread(this);
    timer->setInterval(250);
    connect(timer, SIGNAL(timeout()), SLOT(read()), Qt::DirectConnection);
    timer->start();

    exec();

    delete socket;
    delete timer;
}

void ClientHandler::sendData(const QVariantMap &map)
{
    QDataStream stream(socket);

    QByteArray ba_data=
            QJsonDocument::fromVariant(map).toBinaryData();

    stream << MARKER;
    stream << (qint32)ba_data.size();
    socket->write(ba_data);
    socket->flush();
}

void ClientHandler::cmdProtocolVersion(QVariantMap *vm)
{
    vm->clear();

    vm->insert("msg", Message::ProtocolVersion);
    vm->insert("data", PROTOCOL_VERSION);

    sendData(*vm);
}

void ClientHandler::cmdKeyPressed(QVariantMap *vm)
{
    if(!vm->contains("data"))
        return;

    emit keyPressed(vm->value("data").toInt());
}

void ClientHandler::cmdPlayerSeek(QVariantMap *vm)
{
    if(!vm->contains("data"))
        return;

    emit playerSeek(vm->value("data").toLongLong());
}
