#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QWidget>

class QAbstractVideoSurface;

class VideoWidgetSurface;

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
    VideoWidgetSurface *surface;
};

#endif // VIDEO_WIDGET_H
