/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef FF_ENCODER_THREAD_H
#define FF_ENCODER_THREAD_H

#include <QThread>

#include <atomic>

#include "ff_encoder.h"
#include "frame_buffer.h"

class FFEncoderThread : public QThread
{
    Q_OBJECT

public:
    FFEncoderThread(int enc_num, FFEncoderBaseFilename *base_filename, FFEncStartSync *start_sync, QString store_dir, QString encoding_tool_name, QObject *parent=0);
    ~FFEncoderThread();

    FrameBuffer <Frame::ptr>::ptr frameBuffer();

    bool isWorking();

public slots:
    void setConfig(FFEncoder::Config cfg);
    void stopCoder();

private slots:
    void onStateChanged(bool state);

private:
    FrameBuffer<Frame::ptr>::ptr frame_buffer;

    bool is_working;

    std::atomic <bool> running;

    QString store_dir;
    FFEncoderBaseFilename *base_filename;
    FFEncStartSync *start_sync;
    QString encoding_tool_name;
    int enc_num;

protected:
    void run();

signals:
    void sigSetConfig(FFEncoder::Config cfg);
    void sigStopCoder();

    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);

    void errorString(QString err_string);

    void restartIn();
    void restartOut();
};

#endif // FF_ENCODER_THREAD_H
