#ifndef AUDIO_LEVEL_WIDGET_H
#define AUDIO_LEVEL_WIDGET_H

#include <QWidget>

class FrameBuffer;

class QTimer;

class AudioLevelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioLevelWidget(QWidget *parent=0);

    FrameBuffer *frameBuffer();

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void checkBuffer();

private:
    FrameBuffer *frame_buffer;
    QTimer *timer;

    int16_t level[8];

signals:
    void levels(qint16 l, qint16 r, qint16 c, qint16 lfe, qint16 bl, qint16 br, qint16 sl, qint16 sr);
};


#endif // AUDIO_LEVEL_WIDGET_H
