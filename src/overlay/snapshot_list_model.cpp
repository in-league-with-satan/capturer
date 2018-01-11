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
