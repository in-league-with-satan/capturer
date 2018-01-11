#ifndef FF_ENCODER_BASE_FILENAME_H
#define FF_ENCODER_BASE_FILENAME_H

#include <QMutex>
#include <QString>

class FFEncoderBaseFilename
{
public:
    operator QString();
    QString operator=(const QString &val);

//    void setName(const QString &val);
//    QString getName();

    void clear();
    bool isEmpty();

private:
    QMutex mutex;
    QString str;
};

#endif // FF_ENCODER_BASE_FILENAME_H
