#ifndef OVERLAY_VIEW_H
#define OVERLAY_VIEW_H

#include <QQuickWidget>

class QmlMessenger;

class OverlayView : public QQuickWidget
{
    Q_OBJECT

public:
    OverlayView();
    ~OverlayView();

    void setMessenger(QmlMessenger *messenger);

protected:
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

};

#endif // OVERLAY_VIEW_H
