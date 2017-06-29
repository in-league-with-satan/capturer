#include <QDebug>
#include <QAbstractItemModel>
#include <QQmlEngine>

#include "settings_model.h"

SettingsModel::SettingsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
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

void SettingsModel::setData(const int &index, int role, QVariant data, bool qml)
{
    if(index<0 || index>=d.size())
        return;

//    qInfo() << "SettingsModel::setData" << index << role << data;

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

    emit dataChanged(index, role, qml);
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

void SettingsModel::add(const SettingsModel::Data &data)
{
    beginInsertRows(QModelIndex(), d.size(), d.size());

    d.append(data);

    endInsertRows();
}

