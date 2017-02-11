#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QWidget>

class QAbstractVideoSurface;

class VideoSurface;

class VideoWidget: public QWidget
{
    Q_OBJECT

public:
    VideoWidget(QWidget *parent=0);
    ~VideoWidget();

    QAbstractVideoSurface *videoSurface() const;

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    VideoSurface *surface;
};

#endif // VIDEO_WIDGET_H
