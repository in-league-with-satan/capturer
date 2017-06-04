#include <QDebug>

#include "client_handler.h"

#include "server.h"


Server::Server(quint16 port, QObject *parent)
    : QThread(parent)
    , port(port)
{
    start();
}

Server::~Server()
{
    quit();

    while(isRunning()) {
        msleep(30);
    }
}

void Server::run()
{
    if(port==0)
        return;

    tcp_server=new TcpServer();

    tcp_server->moveToThread(this);

    connect(tcp_server, SIGNAL(newConnection(qintptr)), SLOT(newConnection(qintptr)));

    if(!tcp_server->listen(QHostAddress::Any, port)) {
        qCritical() << tcp_server->errorString();
        return;
    }

    exec();
}

void Server::newConnection(qintptr socket_descriptor)
{
    qInfo() << "client connected" << socket_descriptor;

    ClientHandler *ch=new ClientHandler(socket_descriptor);

    connect(ch, SIGNAL(finished()), ch, SLOT(deleteLater()));
    connect(ch, SIGNAL(keyPressed(int)), SIGNAL(keyPressed(int)), Qt::QueuedConnection);
}
