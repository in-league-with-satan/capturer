#include "image_provider.h"

ImageProvider::ImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requested_size)
{
    if(image.contains(id)) {
        QImage img=image.value(id);

        *size=img.size();

        return img;
    }

    return QQuickImageProvider::requestImage(id, size, requested_size);
}

void ImageProvider::addImage(const QString &id, const QImage &img)
{
    image[id]=img;
}

