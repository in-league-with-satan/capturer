#include <QDebug>
#include <QDataStream>
#include <QJsonDocument>
#include <QMutexLocker>
#include <qcoreapplication.h>

#include "client.h"

Client::Client(QObject *parent)
    : QThread(parent)
{
    start();
}

Client::~Client()
{
    running=false;

    while(isRunning()) {
        msleep(300);
    }
}

void Client::connectToHost(const QString &host, quint16 port)
{
    QMutexLocker ml(&mutex);

    this->host=host;
    this->port=port;
}

void Client::command(int cmd_code)
{
    QMutexLocker ml(&mutex);

    if(protocol_ok || cmd_code==Command::GetProtocolVersion)
        cmd.enqueue(qMakePair(cmd_code, QVariant()));
}

void Client::commandKey(int key_code)
{
    QMutexLocker ml(&mutex);

    if(protocol_ok)
        cmd.enqueue(qMakePair(Command::KeyPressed, key_code));

    else {
        qInfo() << "!protocol_ok";
    }
}

void Client::dropConnection()
{
    socket->disconnectFromHost();

    QMutexLocker ml(&mutex);

    host.clear();
}

void Client::run()
{
    socket=new QTcpSocket();

    socket->moveToThread(this);

    socket->setSocketOption(QAbstractSocket::LowDelayOption, true);
    socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);

    running=true;

    QString local_host_name;
    int local_port;

    while(running) {
        if(socket->state()!=QTcpSocket::ConnectedState && socket->state()!=QTcpSocket::ConnectingState) {
            if(protocol_ok) {
                protocol_ok=false;

                emit recordIsRunning(false);
            }

            {
                QMutexLocker ml(&mutex);

                local_host_name=host;
                local_port=port;
            }

            if(!local_host_name.isEmpty()) {
                socket->connectToHost(local_host_name, local_port);

                if(socket->waitForConnected(5000)) {
                    command(Command::GetProtocolVersion);

                } else {
                    // qInfo() << "!waitForConnected" << socket->errorString();
                }
            }
        }

        if(socket->state()==QTcpSocket::ConnectedState)
            sendRequest();

        if(socket->bytesAvailable()>=8)
            read();

        qApp->processEvents();

        msleep(40);
    }
}

void Client::sendRequest()
{
    int code;
    QVariant data;

    {
        QMutexLocker ml(&mutex);

        if(cmd.isEmpty())
            return;

        auto t=cmd.dequeue();

        code=t.first;
        data=t.second;
    }

    QDataStream stream(socket);

    QVariantMap map;

    map.insert("cmd", code);
    map.insert("data", data);

    QByteArray ba_data=
            QJsonDocument::fromVariant(map).toBinaryData();

    stream << MARKER;
    stream << (qint32)ba_data.size();
    socket->write(ba_data);
    socket->flush();
}

void Client::read()
{
    QDataStream stream(socket);

    quint32 marker;
    qint32 data_size;

    stream >> marker;
    stream >> data_size;

    if(marker!=MARKER) {
        qCritical() << "wrong marker" << marker << MARKER;

        dropConnection();

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

    QVariantMap vm=
        QJsonDocument::fromBinaryData(ba_data, QJsonDocument::BypassValidation).toVariant().toMap();

    switch(vm.value("msg", -1).toInt()) {
    case Message::ProtocolVersion:
        checkProtocol(&vm);
        break;

    case Message::RecStateChanged:
        recStateChanged(&vm);
        break;

    case Message::RecStats:
        recStats(&vm);
        break;

    default:
        break;
    }
}

void Client::checkProtocol(QVariantMap *vm)
{
    if(vm->value("data", -1).toInt()==PROTOCOL_VERSION)
        protocol_ok=true;

    qInfo() << "checkProtocol:" << protocol_ok;
}

void Client::recStateChanged(QVariantMap *vm)
{
    emit recordIsRunning(vm->value("data", false).toBool());
}

void Client::recStats(QVariantMap *vm)
{
    emit recStats(NRecStats().fromExt(vm->value("data").toMap()));
}
