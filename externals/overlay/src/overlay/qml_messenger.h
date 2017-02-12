#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>
#include <QDebug>


class QmlMessenger : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList modelVideoEncoder READ getModelVideoEncoder WRITE setModelVideoEncoder NOTIFY modelVideoEncoderChanged)
    Q_PROPERTY(QStringList modelPixelFormat READ getModelPixelFormat WRITE setModelPixelFormat NOTIFY modelPixelFormatChanged)

public:
    explicit QmlMessenger(QObject *parent=0);

    ~QmlMessenger();

    void recStats(QString duration, QString bitrate, QString size);

    QStringList getModelVideoEncoder() const;
    void setModelVideoEncoder(const QStringList &model);

    QStringList getModelPixelFormat() const;
    void setModelPixelFormat(const QStringList &model);

public slots:
    void onVideoCodecIndexChanged(const int &index);

    void onPixelFormatIndexChanged(const int &index);

    void onCrfChanged(const int &value);

    void onHalfFpsChanged(const bool &value);

    void onStopOnDropChanged(const bool &value);

private:
    QStringList model_video_encoder;
    int model_video_encoder_index;

    QStringList model_pixel_format;
    int model_pixel_format_index;

    int crf;

signals:
    void updateRecStats(QString duration, QString bitrate, QString size);

    void recStarted();
    void recStopped();

    void showMenu();

    void keyPressed(const Qt::Key &key);

    void back();

    void modelVideoEncoderChanged(const QStringList &model, QPrivateSignal);
    void videoEncoderIndexSet(const int &index);

    void modelPixelFormatChanged(const QStringList, QPrivateSignal);
    void pixelFormatIndexSet(const int &index);

    void crfSet(const int &value);

    void halfFpsSet(const bool &value);

    void stopOnDropSet(const bool &value);
};

#endif // QML_MESSENGER_H
