#include <QDebug>
#include <QQmlContext>
#include <QApplication>

#include "qml_messenger.h"


#include "overlay_view.h"

OverlayView::OverlayView() :
    QQuickView()
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    setFormat(format);


    setColor(Qt::transparent);

    setSurfaceType(QSurface::OpenGLSurface);

    setClearBeforeRendering(true);

    setFlags(Qt::FramelessWindowHint);

    // installEventFilter(this);
}

OverlayView::~OverlayView()
{
}

void OverlayView::setMessenger(QmlMessenger *messenger)
{
    rootContext()->setContextProperty("messenger", messenger);
}

void OverlayView::focusOutEvent(QFocusEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void OverlayView::focusInEvent(QFocusEvent *)
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

/*
bool OverlayView::eventFilter(QObject *object, QEvent *event)
{
    if(event->type()==QEvent::KeyPress) {
        return true;

    } else if(event->type()==QEvent::MouseButtonPress) {
        return true;
    }

    return QObject::eventFilter(object, event);
}
*/
