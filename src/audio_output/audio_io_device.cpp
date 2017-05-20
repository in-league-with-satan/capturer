#include <QDebug>
#include <QFile>
#include <QApplication>

#include "audio_io_device.h"


AudioIODevice::AudioIODevice(QObject *parent)
    : QIODevice(parent)
{
//    QFile f;

//    f.setFileName(qApp->applicationDirPath() + "/audio_3.raw");
//    f.open(QFile::ReadOnly);

//    ba_data=f.readAll();

    QIODevice::open(QIODevice::ReadWrite);
}

bool AudioIODevice::open(QIODevice::OpenMode mode)
{
    Q_UNUSED(mode)

    qWarning() << "AudioIODevice::open";

    return true;
}

void AudioIODevice::close()
{
    qWarning() << "AudioIODevice::close";
}

qint64 AudioIODevice::pos() const
{
    return 0;
}

qint64 AudioIODevice::size() const
{
    return ba_data.size();
}

bool AudioIODevice::seek(qint64 pos)
{
    Q_UNUSED(pos)

    return false;
}

bool AudioIODevice::atEnd() const
{
    return ba_data.isEmpty();
}

bool AudioIODevice::reset()
{
    qWarning() << "AudioIODevice::reset";

    return true;
}

bool AudioIODevice::isSequential() const
{
    return true;
}

qint64 AudioIODevice::bytesAvailable() const
{
    return ba_data.size();
}

void AudioIODevice::clear()
{
    ba_data.clear();
}

bool AudioIODevice::canReadLine() const
{
    qWarning() << "AudioIODevice::canReadLine";

    return false;
}

qint64 AudioIODevice::readData(char *data, qint64 maxlen)
{
    if(maxlen>ba_data.size())
        maxlen=ba_data.size();

    if(maxlen>0) {
        memcpy(data, ba_data.data(), maxlen);

        ba_data.remove(0, maxlen);
    }

    return maxlen;
}

qint64 AudioIODevice::writeData(const char *data, qint64 len)
{
    ba_data.append(data, len);

    return len;
}

