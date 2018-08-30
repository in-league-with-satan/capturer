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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileInfo>
#include <qcoreapplication.h>

#include "database.h"

#define OnlyFilename(str) QFileInfo(str).fileName().toLower().toUtf8()

Database::Database(QObject *parent)
    : QObject(parent)
{
    dbname="db_" + QString::number((qintptr)this);

    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE", dbname);

    db.setDatabaseName(qApp->applicationDirPath() + "/capturer.sqlite");

    bool db_state=db.open();

    if(!db_state) {
        qCritical() << "open db err:" << db.lastError().text();
        return;
    }

    tableInit();

    dropOldSnapshots();
}

Database::~Database()
{
}

bool Database::addSnapshot(QString key, const QByteArray &data)
{
    if(!QSqlDatabase::database(dbname).isOpen()) {
        qCritical() << "db is not opened";
        return false;
    }

    removeSnapshot(key);

    QSqlQuery q(QSqlDatabase::database(dbname));

    q.prepare(QStringLiteral("INSERT INTO snapshots(key, timestamp, data) VALUES(?, ?, ?)"));
    q.addBindValue(OnlyFilename(key));
    q.addBindValue(QDateTime::currentSecsSinceEpoch());
    q.addBindValue(data);

    if(!q.exec()) {
        qCritical() << q.lastError().text() << q.lastQuery();
        return false;
    }

    return true;
}

bool Database::snapshot(QString key, QByteArray *data)
{
    if(!QSqlDatabase::database(dbname).isOpen()) {
        qCritical() << "db is not opened";
        return false;
    }

    QSqlQuery q(QSqlDatabase::database(dbname));

    q.prepare(QStringLiteral("SELECT data FROM snapshots WHERE key=?"));
    q.addBindValue(OnlyFilename(key));

    if(!q.exec()) {
        qCritical() << q.lastError().text() << q.lastQuery();
        return false;
    }

    if(!q.next())
        return false;

    (*data)=q.value(0).toByteArray();

    q.prepare(QStringLiteral("UPDATE snapshots SET timestamp=? WHERE key=?"));
    q.addBindValue(QDateTime::currentSecsSinceEpoch());
    q.addBindValue(OnlyFilename(key));
    q.exec();

    return true;
}

bool Database::removeSnapshot(QString key)
{
    if(!QSqlDatabase::database(dbname).isOpen()) {
        qCritical() << "db is not opened";
        return false;
    }

    QSqlQuery q(QSqlDatabase::database(dbname));

    q.prepare(QStringLiteral("DELETE FROM snapshots WHERE key=?"));
    q.addBindValue(OnlyFilename(key));

    if(!q.exec()) {
        qCritical() << q.lastError().text() << q.lastQuery();
        return false;
    }

    return true;
}

void Database::tableInit()
{
    if(!QSqlDatabase::database(dbname).isOpen()) {
        qCritical() << "db is not opened";
        return;
    }

    QSqlQuery q(QSqlDatabase::database(dbname));

    if(!q.exec("CREATE TABLE IF NOT EXISTS snapshots("
               "key BLOB PRIMARY KEY NOT NULL,"
               " timestamp INTEGER NOT NULL,"
               " data BLOB NULL)")) {
        qCritical() << q.lastError().text() << q.lastQuery();
    }
}

void Database::dropOldSnapshots()
{
    if(!QSqlDatabase::database(dbname).isOpen()) {
        qCritical() << "db is not opened";
        return;
    }

    QSqlQuery q(QSqlDatabase::database(dbname));

    q.prepare("DELETE FROM snapshots WHERE timestamp<?");
    q.addBindValue(QDateTime::currentDateTimeUtc().addDays(-7).toSecsSinceEpoch());

    if(!q.exec()) {
        qCritical() << q.lastError().text() << q.lastQuery();
    }
}
