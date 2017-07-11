#include <QDebug>
#include <QTimer>
#include <QDateTime>
//#include <QQmlEngine>
#include <qcoreapplication.h>

#include "image_provider.h"
#include "snapshot_list_model.h"

#include "ff_media_info_thread.h"
#include "ff_snapshot.h"

#include "file_system_model.h"


FileSystemModel::FileSystemModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    image_provider=new ImageProvider();

    media_info=new FFMediaInfoThread(this);
    connect(media_info, SIGNAL(ready(QString,QString)), SLOT(addMediaInfo(QString,QString)), Qt::QueuedConnection);

    snapshot=new FFSnapshot(this);
    connect(snapshot, SIGNAL(ready(QString,QImage)), SLOT(addSnapshot(QString,QImage)), Qt::QueuedConnection);

    filter_ext << "mkv";

    model=new QFileSystemModel(this);
    model->setReadOnly(false);

    model->setFilter(QDir::Dirs | QDir::Files | QDir::NoDot | QDir::NoSymLinks);

    setFilterRole(QFileSystemModel::FilePathRole);

    setSourceModel(model);

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(srcRowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(srcRowsRemoved(QModelIndex,int,int)));
}

FileSystemModel::~FileSystemModel()
{
}

ImageProvider *FileSystemModel::imageProvider()
{
    return image_provider;
}

QModelIndex FileSystemModel::setRootPath(const QString &path)
{
    return mapFromSource(model->setRootPath(path));
}

QString FileSystemModel::rootPath() const
{
    return model->rootPath();
}

QModelIndex FileSystemModel::rootPathIndex() const
{
    return index(rootPath());
}

QString FileSystemModel::path(const QModelIndex &index) const
{
    return index.data(QFileSystemModel::FilePathRole).toString();
}

QModelIndex FileSystemModel::index(const QString &path) const
{
    return mapFromSource(model->index(path));
}

bool FileSystemModel::hasChildren(const QModelIndex &parent) const
{
    return model->hasChildren(mapToSource(parent));
}

bool FileSystemModel::isDir(const QModelIndex &index) const
{
    // qInfo() << "isDir" << path(index) << model->isDir(mapToSource(index));

    return model->isDir(mapToSource(index));
}

bool FileSystemModel::isDir(const QString &path) const
{
    // qInfo() << "isDir 2" << path;

    return model->isDir(model->index(path));
}

QString FileSystemModel::fileSize(const QModelIndex &index) const
{
    const QString filepath=
            index.data(QFileSystemModel::FilePathRole).toString();

    if(filepath.isEmpty())
        return QStringLiteral("0 MB");

    return QString(QLatin1String("%1 MB")).arg(QLocale().toString(QFileInfo(filepath).size()/1024/1024));
}

QString FileSystemModel::ext(const QModelIndex &index) const
{
    return model->fileInfo(mapToSource(index)).suffix().toLower();
}

QString FileSystemModel::mediaInfo(const QModelIndex &index) const
{
    const QString key=index.data(QFileSystemModel::FilePathRole).toString();

    if(!file_media_info.contains(key))
        return QString();

    return file_media_info[key];
}

SnapshotListModel *FileSystemModel::snapshotListModel(const QModelIndex &index)
{
    QFileInfo fi(index.data(QFileSystemModel::FilePathRole).toString());

    if(!filter_ext.contains(fi.suffix(), Qt::CaseInsensitive))
        return nullptr;

    if(!file_snapshot_list_model.contains(fi.filePath()))
        file_snapshot_list_model.insert(fi.filePath(), new SnapshotListModel());

    return file_snapshot_list_model[fi.filePath()];
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    // qInfo() << "FileSystemModel::data" << role;

    return QSortFilterProxyModel::data(index, role);
}

void FileSystemModel::srcRowsInserted(const QModelIndex &parent, int first, int last)
{
    for(int i=first; i<=last; ++i) {
        const QModelIndex idx=model->index(i, 0, parent);

        const QFileInfo file_info=model->fileInfo(idx);

        if(!file_info.isFile())
            continue;

        if(!filter_ext.contains(file_info.suffix(), Qt::CaseInsensitive))
            continue;

        if(file_media_info.contains(file_info.filePath()))
            continue;

        snapshot->enqueue(file_info.filePath());
        media_info->enqueue(file_info.filePath());
    }
}

void FileSystemModel::srcRowsRemoved(const QModelIndex &parent, int first, int last)
{
    for(int i=first; i<=last; ++i) {
        const QModelIndex idx=model->index(i, 0, parent);

        const QFileInfo file_info=model->fileInfo(idx);

        if(!file_info.isFile())
            continue;

        if(!filter_ext.contains(file_info.suffix(), Qt::CaseInsensitive))
            continue;

        file_media_info.remove(file_info.filePath());

        image_provider->removeImages(file_info.filePath());

        snapshot->enqueueRemove(file_info.filePath());

        if(file_snapshot_list_model.contains(file_info.filePath())) {
            file_snapshot_list_model[file_info.filePath()]->deleteLater();
            file_snapshot_list_model.remove(file_info.filePath());
        }
    }
}

void FileSystemModel::addMediaInfo(QString key, QString info)
{
    file_media_info[key]=info;

    emit changed(this);
}

void FileSystemModel::addSnapshot(QString key, QImage image)
{
    SnapshotListModel *mdl=nullptr;

    if(file_snapshot_list_model.contains(key)) {
        mdl=file_snapshot_list_model[key];

    } else {
        mdl=new SnapshotListModel();
        file_snapshot_list_model.insert(key, mdl);
    }

    QString id=QString("%1.%2").arg(key).arg(mdl->rowCount());

    image_provider->addImage(id, image);

    mdl->add(id);
}

void FileSystemModel::disableSnapshots(bool value)
{
    media_info->pause(value);
    snapshot->pause(value);
}

bool FileSystemModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(source_parent.isValid()) {
        QModelIndex idx=model->index(source_row, 0,  source_parent);

        if(source_parent.data(QFileSystemModel::FilePathRole).toString()==rootPath()) {
            if(idx.data(QFileSystemModel::FileNameRole).toString()=="..")
                return false;
        }

        const QFileInfo file_info=model->fileInfo(idx);

        if(file_info.isFile())
            if(!filter_ext.contains(file_info.suffix(), Qt::CaseInsensitive))
                return false;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool FileSystemModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // if sorting by file names column
    if(sortColumn()==0) {
        const bool asc=sortOrder()==Qt::AscendingOrder ? true : false;

        const QFileInfo left_file_info=model->fileInfo(left);
        const QFileInfo right_file_info=model->fileInfo(right);

        // if DotAndDot move in the beginning
        if(sourceModel()->data(left).toString()==QStringLiteral(".."))
            return asc;

        if(sourceModel()->data(right).toString()==QStringLiteral(".."))
            return !asc;

        // move dirs upper
        if(!left_file_info.isDir() && right_file_info.isDir())
            return !asc;

        if(left_file_info.isDir() && !right_file_info.isDir())
            return asc;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

