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

#ifndef FF_SNAPSHOT_H
#define FF_SNAPSHOT_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QPixmap>

#include <atomic>

class QTimer;
class Database;

class FFSnapshot : public QThread
{
    Q_OBJECT

public:
    FFSnapshot(QObject *paretn=0);

public slots:
    void enqueue(const QString &filename);

    void pause(bool state);
    void viewerVisible(bool value);

protected:
    void run();

private slots:
    void checkQueue();

private:
    QTimer *timer;

    bool accurate_seek;
    int shots_per_10_min;

    QQueue <QString> queue;

    QMutex mutex_queue;

    std::atomic <bool> on_pause;
    std::atomic <bool> viewer_visible;

    Database *database;

signals:
    void ready(QString key, QImage image);
};

#endif // FF_SNAPSHOT_H
