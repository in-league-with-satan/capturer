#include <QMutexLocker>

#include "ff_encoder_base_filename.h"

FFEncoderBaseFilename::operator QString()
{
    QMutexLocker ml(&mutex);

    return str;
}

QString FFEncoderBaseFilename::operator=(const QString &val)
{
    QMutexLocker ml(&mutex);

    str=val;

    return str;
}

void FFEncoderBaseFilename::clear()
{
    QMutexLocker ml(&mutex);

    str.clear();
}

bool FFEncoderBaseFilename::isEmpty()
{
    QMutexLocker ml(&mutex);

    return str.isEmpty();
}
