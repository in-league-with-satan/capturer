#ifndef AUDIO_LEVEL_WIDGET_H
#define AUDIO_LEVEL_WIDGET_H

#include <QWidget>

class AudioLevelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioLevelWidget(QWidget *parent=0);

public slots:
    void write(const QByteArray &data, int channels, int sample_size);

protected:
    void paintEvent(QPaintEvent *event);

private slots:

private:
    int32_t level[8];

    bool sample_size_16;
};


#endif // AUDIO_LEVEL_WIDGET_H
