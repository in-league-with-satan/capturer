#ifndef OVERLAY_VIEW_H
#define OVERLAY_VIEW_H

#include <QQuickView>

class QmlMessenger;

class OverlayView : public QQuickView
{
    Q_OBJECT

public:
    OverlayView();
    ~OverlayView();

    void setMessenger(QmlMessenger *messenger);

private:

private slots:

};

#endif // OVERLAY_VIEW_H
