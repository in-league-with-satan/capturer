/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef AUDIO_IO_DEVICE_H
#define AUDIO_IO_DEVICE_H

#include <QIODevice>


class AudioIODevice: public QIODevice
{
    Q_OBJECT

public:
    AudioIODevice(QObject *parent=0);

    virtual bool open(OpenMode mode);
    virtual void close();

    virtual qint64 pos() const;
    virtual qint64 size() const;
    virtual bool seek(qint64 pos);
    virtual bool atEnd() const;
    virtual bool reset();

    virtual bool isSequential() const;

    virtual qint64 bytesAvailable() const;

    void clear();

    virtual bool canReadLine() const;

private:
    QByteArray ba_data;

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);
};

#endif // AUDIO_IO_DEVICE_H
