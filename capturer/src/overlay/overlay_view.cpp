/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QQmlContext>
#include <QApplication>
#include <QQmlEngine>

#include "qml_messenger.h"

#include "overlay_view.h"


OverlayView::OverlayView(QWidget *parent)
    : QQuickWidget(parent)
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    format.setStencilBufferSize(8);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::DefaultSwapBehavior);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    // setFormat(format);

    // setAttribute(Qt::WA_TranslucentBackground, true);
    // setAttribute(Qt::WA_AlwaysStackOnTop, true);

    setClearColor(Qt::black);

    setResizeMode(QQuickWidget::SizeRootObjectToView);
}

OverlayView::~OverlayView()
{
}

void OverlayView::setMessenger(QmlMessenger *messenger)
{
    rootContext()->setContextProperty(QStringLiteral("messenger"), messenger);
}

void OverlayView::addImageProvider(const QString &id, QQmlImageProviderBase *image_provider)
{
    engine()->addImageProvider(id, image_provider);
}

void OverlayView::focusOutEvent(QFocusEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void OverlayView::focusInEvent(QFocusEvent *)
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
}
