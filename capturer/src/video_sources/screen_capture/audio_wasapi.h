/******************************************************************************

Copyright © 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef AUDIO_WASAPI_H
#define AUDIO_WASAPI_H

#include <QObject>
#include <QList>

#include "protect.h"

class AudioWasapiPrivate;
class AudioConverter;

class AudioWasapi : public QObject
{
    Q_OBJECT

public:
    AudioWasapi(QObject *parent=0);
    ~AudioWasapi();

    struct Device {
        void *d;
        QString name;
    };

    QList <Device> availableAudioInput() const;
    QStringList availableAudioInputStr();

    bool deviceStart(const Device &device);
    void deviceStop();

    int channels() const;
    int sampleSize() const;

    QByteArray getData();

public slots:
    void updateDevList();

private:
    QList <AudioWasapi::Device> list_devices;
    Protect <QStringList> list_devices_str;

    AudioWasapiPrivate *d;
    AudioConverter *audio_converter;
};

#endif // AUDIO_WASAPI_H
