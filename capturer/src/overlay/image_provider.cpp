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
#include "image_provider.h"

ImageProvider::ImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requested_size)
{
    if(image.contains(id)) {
        QImage img=image.value(id);

        (*size)=img.size();

        return img;
    }

    return QQuickImageProvider::requestImage(id, size, requested_size);
}

void ImageProvider::addImage(const QString &id, const QImage &img)
{
    image[id]=img;
}

void ImageProvider::removeImages(const QString &id)
{
    foreach(const QString &key, image.keys()) {
        if(key.startsWith(id))
            image.remove(key);
    }
}

void ImageProvider::clear()
{
    image.clear();
}
