#ifndef FILE_SYSTEM_MODEL_H
#define FILE_SYSTEM_MODEL_H

#include <QFileSystemModel>
#include <QSortFilterProxyModel>

class FFMediaInfoThread;
class FFSnapshot;
class ImageProvider;
class SnapshotListModel;

class FileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FileSystemModel(QObject *parent=0);
    ~FileSystemModel();

    ImageProvider *imageProvider();

    Q_INVOKABLE QModelIndex index(const QString &path) const;
    Q_INVOKABLE QModelIndex setRootPath(const QString &path);

    Q_INVOKABLE QString rootPath() const;
    Q_INVOKABLE QModelIndex rootPathIndex() const;

    Q_INVOKABLE QString path(const QModelIndex &index) const;

    Q_INVOKABLE bool hasChildren(const QModelIndex &parent=QModelIndex()) const;

    Q_INVOKABLE bool isDir(const QModelIndex &index) const;
    Q_INVOKABLE bool isDir(const QString &path) const;

    Q_INVOKABLE QString fileSize(const QModelIndex &index) const;

    Q_INVOKABLE QString ext(const QModelIndex &index) const;

    Q_INVOKABLE QString mediaInfo(const QModelIndex &index) const;

    Q_INVOKABLE SnapshotListModel *snapshotListModel(const QModelIndex &index);

    enum Roles {
        MediaInfoRole=QFileSystemModel::FilePermissions + 1
    };

public slots:
    void disableSnapshots(bool value);

private slots:
    void srcRowsInserted(const QModelIndex &parent, int first, int last);
    void addMediaInfo(QString key, QString info);
    void addSnapshot(QString key, QImage image);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    QFileSystemModel *model;

    QStringList filter_ext;

    QHash <QString, QString> file_media_info;
    QHash <QString, SnapshotListModel*> file_snapshot_list_model;

    ImageProvider *image_provider;

    FFMediaInfoThread *media_info;
    FFSnapshot *snapshot;

signals:
    void changed(FileSystemModel *model);

    void playMedia(const QString &filename);
};

#endif // FILE_SYSTEM_MODEL_H
