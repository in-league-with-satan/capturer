/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

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
    void clear();

private:
    QStringList id_list;
};

#endif // SNAPSHOT_LIST_MODEL_H
