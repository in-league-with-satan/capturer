#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>
#include <QDebug>
#include <QFileSystemModel>

#include "file_system_model.h"
#include "snapshot_list_model.h"

class QmlMessenger : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList modelVideoEncoder READ getModelVideoEncoder WRITE setModelVideoEncoder NOTIFY modelVideoEncoderChanged)
    Q_PROPERTY(QStringList modelPixelFormat READ getModelPixelFormat WRITE setModelPixelFormat NOTIFY modelPixelFormatChanged)
    Q_PROPERTY(FileSystemModel* fileSystemModel READ fileSystemModel NOTIFY fileSystemModelChanged)

    Q_PROPERTY(QString rootPath READ getRootPath NOTIFY fileSystemModelChanged)

public:
    explicit QmlMessenger(QObject *parent=0);
    ~QmlMessenger();

    QStringList getModelVideoEncoder() const;
    void setModelVideoEncoder(const QStringList &model);

    QStringList getModelPixelFormat() const;
    void setModelPixelFormat(const QStringList &model);

    FileSystemModel *fileSystemModel();

    QString getRootPath();

public slots:
    void keyEvent(const Qt::Key &key);

private slots:
    void checkFreeSpace();

private:
    QStringList model_video_encoder;
    int model_video_encoder_index;

    QStringList model_pixel_format;
    int model_pixel_format_index;

    FileSystemModel *file_system_model;

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

    void showHideInfo();

    void showHidePlayerState();
    void showPlayerState(bool visible);

    void showHideDetailedRecState();

    void showFileBrowser();

    void keyPressed(const Qt::Key &key);

    void back();

    void modelVideoEncoderChanged(const QStringList &model, QPrivateSignal);
    void videoEncoderIndexSet(const int &index);

    void modelPixelFormatChanged(const QStringList, QPrivateSignal);
    void pixelFormatIndexSet(const int &index);

    void fileSystemModelChanged(FileSystemModel *model);

    void crfSet(const int &value);

    void halfFpsSet(const bool &value);

    void stopOnDropSet(const bool &value);

    void videoCodecIndexChanged(const int &index);
    void pixelFormatIndexChanged(const int &index);
    void crfChanged(const int &value);
    void halfFpsChanged(const bool &value);
    void stopOnDropChanged(const bool &value);

    void signalLost(const bool &value);

    void playerDurationChanged(const qint64 &duration);
    void playerPositionChanged(const qint64 &position);
    void playerSetPosition(const qint64 &position);

};

#endif // QML_MESSENGER_H
