#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QThread>

class QAudioOutput;
class QIODevice;

class AudioOutputThread : public QThread
{
    Q_OBJECT

public:
    AudioOutputThread(QWidget *parent=0);
    ~AudioOutputThread();

private:
    void run();

    QAudioOutput *audio_output;
    QIODevice *dev_audio_output;

public slots:
    void onInputFrameArrived(QByteArray ba_data);

};

#endif // AUDIO_OUTPUT_THREAD_H
