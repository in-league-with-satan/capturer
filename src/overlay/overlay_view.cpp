#include <QDebug>
#include <QQmlContext>
#include <QApplication>

#include "qml_messenger.h"


#include "overlay_view.h"

OverlayView::OverlayView() :
    QQuickWidget()
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    format.setStencilBufferSize(8);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    // setFormat(format);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_AlwaysStackOnTop, true);

    setClearColor(Qt::transparent);

    setResizeMode(QQuickWidget::SizeRootObjectToView);
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
