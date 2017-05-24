#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>
#include <QFileSystemModel>

#include "settings_model.h"
#include "file_system_model.h"
#include "snapshot_list_model.h"

class QmlMessenger : public QObject
{
    Q_OBJECT

    Q_PROPERTY(SettingsModel* settingsModel READ settingsModel NOTIFY settingsModelChanged)
    Q_PROPERTY(FileSystemModel* fileSystemModel READ fileSystemModel NOTIFY fileSystemModelChanged)

    Q_PROPERTY(QString rootPath READ getRootPath NOTIFY fileSystemModelChanged)

public:
    explicit QmlMessenger(QObject *parent=0);
    ~QmlMessenger();

    Q_INVOKABLE SettingsModel *settingsModel();
    Q_INVOKABLE FileSystemModel *fileSystemModel();

    Q_INVOKABLE QString getRootPath();

    Q_INVOKABLE QString versionThis() const;
    Q_INVOKABLE QString versionLibAVUtil() const;
    Q_INVOKABLE QString versionlibAVCodec() const;
    Q_INVOKABLE QString versionlibAVFormat() const;
    Q_INVOKABLE QString versionlibAVFilter() const;
    Q_INVOKABLE QString versionlibSWScale() const;
    Q_INVOKABLE QString versionlibSWResample() const;

public slots:
    void keyEvent(const Qt::Key &key);
    void setRecStarted(bool value);

private slots:
    void checkFreeSpace();

private:
    SettingsModel *settings_model;
    FileSystemModel *file_system_model;

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

    void showHideAbout();

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

    void settingsModelChanged(SettingsModel *model);
    void fileSystemModelChanged(FileSystemModel *model);

    void signalLost(const bool &value);

    void playerDurationChanged(const qint64 &duration);
    void playerPositionChanged(const qint64 &position);
    void playerSetPosition(const qint64 &position);

};

#endif // QML_MESSENGER_H
