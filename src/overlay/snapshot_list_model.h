#ifndef SNAPSHOT_LIST_MODEL_H
#define SNAPSHOT_LIST_MODEL_H

#include <QAbstractListModel>
#include <QStringList>

class SnapshotListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SnapshotListModel(QObject *parent=0);

    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QHash <int, QByteArray> roleNames() const;

    void add(const QString &id);

private:
    QStringList id_list;
};

#endif // SNAPSHOT_LIST_MODEL_H
