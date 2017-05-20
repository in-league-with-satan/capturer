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
