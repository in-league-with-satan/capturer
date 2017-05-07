#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider();

    virtual QImage requestImage(const QString &id, QSize *size, const QSize& requested_size);

    void addImage(const QString &id, const QImage &img);

private:
    QHash <QString, QImage> image;

};
#endif // IMAGE_PROVIDER_H
