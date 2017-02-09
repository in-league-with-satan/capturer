#include <QDebug>

#include "qml_messenger.h"

QmlMessenger::QmlMessenger(QObject *parent) :
    QObject(parent)
{
}

QmlMessenger::~QmlMessenger()
{
}

void QmlMessenger::recStats(QString duration, QString bitrate, QString size)
{
    emit updateRecStats(duration,  bitrate,  size);
}

QStringList QmlMessenger::getModelVideoEncoder() const
{
    return model_video_encoder;
}

void QmlMessenger::setModelVideoEncoder(const QStringList &model)
{
    model_video_encoder=model;

    emit modelVideoEncoderChanged(model, QPrivateSignal());
}

QStringList QmlMessenger::getModelPixelFormat() const
{
    return model_pixel_format;
}

void QmlMessenger::setModelPixelFormat(const QStringList &model)
{
    model_pixel_format=model;

    emit modelPixelFormatChanged(model, QPrivateSignal());
}

void QmlMessenger::onVideoCodecIndexChanged(const int &index)
{
    qInfo() << "VideoCodecIndexChanged" << index;

    model_video_encoder_index=index;
}

void QmlMessenger::onPixelFormatIndexChanged(const int &index)
{
    qInfo() << "PixelFormatIndexChanged" << index;

    model_pixel_format_index=index;
}

void QmlMessenger::onCrfChanged(const int &value)
{
    qInfo() << "CrfChanged" << value;

    this->crf=value;
}

void QmlMessenger::onHalfFpsChanged(const bool &value)
{
    qInfo() << "HalfFpsChanged" << value;
}

void QmlMessenger::onStopOnDropChanged(const bool &value)
{
    qInfo() << "StopOnDropChanged" << value;
}

