/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include "settings_model.h"

SettingsModel::SettingsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

QVariant SettingsModel::valueData(int *ptr_value, QVariant default_value)
{
    if(!ptr_value)
        return default_value;

    if((*ptr_value)<0)
        return default_value;

    Data *dp=data_p(ptr_value);

    if(!dp)
        return default_value;

    if(dp->values_data.isEmpty())
        return default_value;

    if(dp->values_data.size()<=(*ptr_value))
        return default_value;

    return dp->values_data[*ptr_value];
}

int SettingsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return d.size();
}

QVariant SettingsModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    return data(index.row(), role);
}

QVariant SettingsModel::data(const int &index, int role) const
{
    if(index>=d.size())
        return QVariant();

    switch(role) {
    case Role::type:
        return d[index].type;

    case Role::group:
        return d[index].group;

    case Role::priority:
        return d[index].priority;

    case Role::values:
        return d[index].values;

    case Role::values_data:
        return d[index].values_data;

    case Role::value:
        return *d[index].value;

    // case Qt::DisplayRole:
    case Role::name:
        return d[index].name;

    default:
        ;
    }

    return QVariant();
}

QVariant SettingsModel::value(const int &index)
{
    QVariantList value=data(index, Role::values).toList();

    int index_value=data(index, Role::value).toInt();

    if(index_value<0 || value.size()<=index_value)
        return QVariant();

    return value[index_value];
}

void SettingsModel::setData(const int &index, int role, QVariant data, bool qml, bool block_signal)
{
    if(index<0 || index>=d.size())
        return;


    switch(role) {
    case Role::type:
        d[index].type=data.toInt();
        break;

    case Role::group:
        d[index].group=data.toString();
        break;

    case Role::values:
        d[index].values=data.toStringList();
        break;

    case Role::values_data:
        d[index].values_data=data.toList();
        break;

    case Role::value:
        *d[index].value=data.toInt();
        break;

    case Role::name:
        d[index].name=data.toString();
        break;

    default:
        ;
    }

    if(!block_signal) {
        emit dataChanged(index, role, qml);
    }
}

void SettingsModel::setData(int *ptr_value, int role, QVariant data, bool qml, bool block_signal)
{
    for(int i=0; i<rowCount(); ++i) {
        if(data_p(i)->value==ptr_value) {
            setData(i, role, data, qml, block_signal);
            break;
        }
    }
}

int SettingsModel::focusPrev(int index) const
{
    while(true) {
        index--;

        if(index<0)
            index=d.size() - 1;

        if(d[index].type!=Type::title && d[index].type!=Type::divider)
            break;
    }

    return index;
}

int SettingsModel::focusNext(int index) const
{
    if(d.isEmpty())
        return -1;

    bool reset=false;

    while(true) {
        index++;

        if(index>=d.size()) {
            if(reset)
                return -1;

            reset=true;
            index=0;
        }

        if(d[index].type!=Type::title && d[index].type!=Type::divider)
            break;
    }

    return index;
}

bool SettingsModel::posCheck(int index) const
{
    if(index<0 || index>=d.size())
        return false;

    if(d[index].type!=Type::title && d[index].type!=Type::divider)
        return true;

    return false;
}

void SettingsModel::reload()
{
    emit changed(this);
}

SettingsModel::Data *SettingsModel::data_p(const int &index)
{
    if(index>=d.size())
        return nullptr;

    return &d[index];
}

SettingsModel::Data *SettingsModel::data_p(int *value)
{
    for(int i=0; i<d.size(); ++i)
        if(d[i].value==value)
            return &d[i];

    return nullptr;
}

int SettingsModel::data_p_index(int *value)
{
    for(int i=0; i<d.size(); ++i)
        if(d[i].value==value)
            return i;

    return -1;
}

QHash <int, QByteArray> SettingsModel::roleNames() const
{
    static QHash <int, QByteArray> roles;

    if(roles.isEmpty()) {
        roles[Role::type]=QByteArrayLiteral("type");
        roles[Role::group]=QByteArrayLiteral("group");
        roles[Role::values]=QByteArrayLiteral("values");
        roles[Role::values_data]=QByteArrayLiteral("values_data");
        roles[Role::value]=QByteArrayLiteral("value");
        roles[Role::name]=QByteArrayLiteral("name");
    }

    return roles;
}

void SettingsModel::updateQml()
{
    endResetModel();
}

int SettingsModel::add(const SettingsModel::Data &data)
{
    beginInsertRows(QModelIndex(), d.size(), d.size());

    d.append(data);

    endInsertRows();

    return d.size() - 1;
}

int SettingsModel::add(const QList <SettingsModel::Data> &data)
{
    beginInsertRows(QModelIndex(), d.size(), d.size() + data.size() - 1);

    d.append(data);

    endInsertRows();

    return d.size() - 1;
}

int SettingsModel::insert(int *ptr_value_pos, const SettingsModel::Data &data)
{
    for(int i=0; i<d.size(); ++i) {
        if(d[i].value==ptr_value_pos) {
            beginInsertRows(QModelIndex(), i + 1, i + 1);

            d.insert(i + 1, data);

            endInsertRows();

            return i + 1;
        }
    }

    return 0;
}

int SettingsModel::insert(int *ptr_value_pos, const QList <SettingsModel::Data> &data)
{
    for(int i=0; i<d.size(); ++i) {
        if(d[i].value==ptr_value_pos) {
            beginInsertRows(QModelIndex(), i + 1, i + data.size() );

            for(int j=data.size() - 1; j>=0; --j) {
                d.insert(i + 1, data[j]);
            }

            endInsertRows();

            return i + data.size();
        }
    }

    return 0;
}

void SettingsModel::insert(int pos, const QList <SettingsModel::Data> &data)
{
    beginInsertRows(QModelIndex(), pos + data.size(), pos + data.size());

    for(int j=data.size() - 1; j>=0; --j) {
        d.insert(pos, data[j]);
    }

    endInsertRows();
}

void SettingsModel::removeRow(int *ptr_value)
{
    for(int i=0; i<d.size(); ++i) {
        if(d[i].value==ptr_value) {
            beginRemoveRows(QModelIndex(), i, i);

            d.removeAt(i);

            endRemoveRows();

            return;
        }
    }
}

void SettingsModel::removeGroup(QString group)
{
    for(int i=0; i<d.size(); ++i) {
        if(d[i].group==group) {
            beginRemoveRows(QModelIndex(), i, i);

            d.removeAt(i--);

            endRemoveRows();
        }
    }
}

