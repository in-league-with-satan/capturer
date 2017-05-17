#ifndef OVERLAY_VIEW_H
#define OVERLAY_VIEW_H

#include <QQuickWidget>

class QmlMessenger;
class QQmlImageProviderBase;

class OverlayView : public QQuickWidget
{
    Q_OBJECT

public:
    OverlayView();
    ~OverlayView();

    void setMessenger(QmlMessenger *messenger);

    void addImageProvider(const QString &id, QQmlImageProviderBase *image_provider);

protected:
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

};

#endif // OVERLAY_VIEW_H
