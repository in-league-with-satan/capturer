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

    QStringList getModelVideoEncoder() const;
    void setModelVideoEncoder(const QStringList &model);

    QStringList getModelPixelFormat() const;
    void setModelPixelFormat(const QStringList &model);

public slots:
    void keyEvent(const Qt::Key &key);

private slots:
    void checkFreeSpace();

private:
    QStringList model_video_encoder;
    int model_video_encoder_index;

    QStringList model_pixel_format;
    int model_pixel_format_index;

    int crf;

signals:
    void updateRecStats(QString duration=QString(), QString bitrate=QString(), QString size=QString(),
                        QString buffer_state=QString(), QString dropped_frames_counter=QString());

    void formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format);

    void audioLevels(qint16 l, qint16 r, qint16 c, qint16 lfe, qint16 bl, qint16 br, qint16 sl, qint16 sr);


    void freeSpace(QString size);

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

    void videoCodecIndexChanged(const int &index);
    void pixelFormatIndexChanged(const int &index);
    void crfChanged(const int &value);
    void halfFpsChanged(const bool &value);
    void stopOnDropChanged(const bool &value);
};

#endif // QML_MESSENGER_H
