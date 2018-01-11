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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariantMap>
#include <QMutex>

#define settings Settings::instance()

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings *createInstance(QObject *parent=0);
    static Settings *instance();

    bool load();
    bool save();

    QString host();
    void setHost(const QString &value);

    quint16 port();
    void setPort(const quint16 &value);

    QString routingKey();
    void setRoutingKey(const QString &value);

private:
    struct Main {
        QString host;
        quint16 port;
        QString routing_key;

    } main;

    Settings(QObject *parent=0);

    static Settings *_instance;

    QByteArray ba_hash_file;

    QMutex mutex;
};

#endif // SETTINGS_H
