/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>

#include "store_location.h"

StoreLocation *StoreLocation::_instance=nullptr;

StoreLocation *StoreLocation::createInstance()
{
    if(_instance==nullptr)
        _instance=new StoreLocation();

    return _instance;
}

StoreLocation *StoreLocation::instance()
{
    return _instance;
}

QString StoreLocation::config() const
{
    return loc_config;
}

QString StoreLocation::temp() const
{
    return loc_temp;
}

QString StoreLocation::videos() const
{
    return loc_videos;
}

StoreLocation::StoreLocation()
{
    bool portable_mode=false;
    QString portable_path;

    foreach(QString arg, qApp->arguments()) {
        if(arg.startsWith("--portable-mode")) {
            portable_mode=true;

            if(arg.contains("=")) {
                portable_path=arg.split("=", QString::SkipEmptyParts).last();
            }
        }
    }

    if(!portable_path.isEmpty()) {
        if(QDir::isRelativePath(portable_path)) {
            loc_config=QDir(qApp->applicationDirPath()).filePath(portable_path);
            loc_temp=QDir(qApp->applicationDirPath()).filePath(portable_path);
            loc_videos=QDir(QDir(qApp->applicationDirPath()).filePath(portable_path)).filePath("videos");

        } else {
            loc_config=portable_path;
            loc_temp=portable_path;
            loc_videos=QDir(portable_path).filePath("videos");
        }

        if(mkPaths())
            return;
    }

    if(portable_mode) {
        loc_config=qApp->applicationDirPath();
        loc_temp=qApp->applicationDirPath();
        loc_videos=QDir(qApp->applicationDirPath()).filePath("videos");

        if(mkPaths())
            return;
    }

    loc_config=QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).filePath("capturer");
    loc_temp=QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath("capturer");
    loc_videos=QDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).filePath("capturer");


    if(!mkPaths()) {
        exit(403);
    }
}

bool StoreLocation::mkPaths()
{
    qDebug() << "loc_config:" << loc_config;
    qDebug() << "loc_temp:" << loc_temp;
    qDebug() << "loc_videos:" << loc_videos;

    if(!QDir().mkpath(loc_config)) {
        qCritical() << "err mkpath:" << loc_config;
        return false;
    }

    if(!QDir().mkpath(loc_temp)) {
        qCritical() << "err mkpath:" << loc_temp;
        return false;
    }

    /*
    if(!QDir().mkpath(loc_videos)) {
        qCritical() << "err mkpath:" << loc_videos;
        return false;
    }
    */

    return true;
}

