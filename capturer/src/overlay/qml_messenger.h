/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>
#include <QFileSystemModel>

#include "settings_model.h"
#include "file_system_model.h"
#include "snapshot_list_model.h"
#include "quick_video_source.h"

class QmlMessenger : public QObject
{
    Q_OBJECT

    Q_PROPERTY(SettingsModel* settingsModel READ settingsModel NOTIFY settingsModelChanged)
    Q_PROPERTY(FileSystemModel* fileSystemModel READ fileSystemModel NOTIFY fileSystemModelChanged)

    Q_PROPERTY(QString rootPath READ getRootPath NOTIFY fileSystemModelChanged)

public:
    explicit QmlMessenger(SettingsModel *settings_model, QObject *parent=0);
    ~QmlMessenger();

    Q_INVOKABLE SettingsModel *settingsModel();
    Q_INVOKABLE FileSystemModel *fileSystemModel();

    Q_INVOKABLE void fileBrowserVisibleState(bool visible);

    Q_INVOKABLE QString getRootPath();

    Q_INVOKABLE QString versionThis() const;
    Q_INVOKABLE QString versionLibAVUtil() const;
    Q_INVOKABLE QString versionlibAVCodec() const;
    Q_INVOKABLE QString versionlibAVFormat() const;
    Q_INVOKABLE QString versionlibAVFilter() const;
    Q_INVOKABLE QString versionlibSWScale() const;
    Q_INVOKABLE QString versionlibSWResample() const;
    Q_INVOKABLE QString networkAddressesStr() const;
    Q_INVOKABLE QStringList networkAddresses() const;

    Q_INVOKABLE QuickVideoSource *videoSourcePrimary();
    Q_INVOKABLE QuickVideoSource *videoSourceSecondary();

public slots:
    void keyEvent(const Qt::Key &key);
    void setRecStarted(bool value);

private slots:

private:
    SettingsModel *settings_model;
    FileSystemModel *file_system_model;

    QuickVideoSource *video_source_primary;
    QuickVideoSource *video_source_secondary;

signals:
    void updateRecStats(QString duration=QString(), QString bitrate=QString(), QString size=QString(),
                        QString buffer_state=QString(), QString dropped_frames_counter=QString());

    void formatChanged(QString format);
    void temperatureChanged(double temperature);

    void audioLevelPrimary(qreal l, qreal r, qreal c, qreal lfe, qreal rl, qreal rr, qreal sl, qreal sr);
    void audioLevelSecondary(qreal l, qreal r, qreal c, qreal lfe, qreal rl, qreal rr, qreal sl, qreal sr);

    void freeSpaceStr(QString size);
    void freeSpace(qint64 size);

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

    void setHdrToSdrEnabled(bool value);
    void setHdrBrightness(double value);
    void setHdrSaturation(double value);

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
    void errorString(QString error_string);

    void previewSecondary(bool visible);
    void previewSecondaryChangePosition();

    void focusReset();
};

#endif // QML_MESSENGER_H
