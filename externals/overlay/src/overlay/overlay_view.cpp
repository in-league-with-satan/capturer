#include <QQmlContext>

#include "qml_messenger.h"


#include "overlay_view.h"

OverlayView::OverlayView() :
    QQuickView()
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    format.setRenderableType(QSurfaceFormat::OpenGL);

    setFormat(format);


    QColor color;
    color.setRedF(0.);
    color.setGreenF(0.);
    color.setBlueF(0.);
    color.setAlphaF(0.);

    setColor(color);


    setSurfaceType(QSurface::OpenGLSurface);

    setClearBeforeRendering(true);

    setFlags(Qt::FramelessWindowHint);
}

OverlayView::~OverlayView()
{
}

void OverlayView::setMessenger(QmlMessenger *messenger)
{
    rootContext()->setContextProperty("messenger", messenger);
}
