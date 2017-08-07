#include <QDebug>
#include <QAbstractItemModel>
#include <QQmlEngine>

#include "snapshot_list_model.h"

SnapshotListModel::SnapshotListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

int SnapshotListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return id_list.size();
}

QVariant SnapshotListModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    switch(role) {
    case Qt::DisplayRole:
        return id_list.value(index.row(), QString());

    default:
        ;
    }

    return QVariant();
}

QHash <int, QByteArray> SnapshotListModel::roleNames() const
{
    QHash <int, QByteArray> roles;

    roles[Qt::DisplayRole]="id";

    return roles;
}

void SnapshotListModel::add(const QString &id)
{
    beginInsertRows(QModelIndex(), id_list.size(), id_list.size());

    id_list.append(id);

    endInsertRows();
}

void SnapshotListModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, id_list.size());

    id_list.clear();

    endRemoveRows();
}
