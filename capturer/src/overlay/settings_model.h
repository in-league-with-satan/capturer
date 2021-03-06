/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef SETTINGS_MODEL_H
#define SETTINGS_MODEL_H

#include <QAbstractListModel>
#include <QStringList>

class SettingsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SettingsModel(QObject *parent=0);

    struct Data {
        Data() {
            type=0;
            value=0;
            priority=Priority::low;
        }

        int type;
        int priority;
        QStringList values;
        QVariantList values_data;
        int *value;
        QString group;
        QString name;
    };

    QVariant valueData(int *ptr_value, QVariant default_value=QVariant());
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(const int &index, int role) const;
    Q_INVOKABLE QVariant value(const int &index);
    Q_INVOKABLE void setData(const int &index, int role, QVariant data, bool qml=false, bool block_signal=false);
    Q_INVOKABLE void setData(int *ptr_value, int role, QVariant data, bool qml=false, bool block_signal=false);

    Q_INVOKABLE int focusPrev(int index) const;
    Q_INVOKABLE int focusNext(int index) const;
    Q_INVOKABLE bool posCheck(int index) const;

    Q_INVOKABLE void reload();

    virtual SettingsModel::Data *data_p(const int &index);
    virtual SettingsModel::Data *data_p(int *value);
    virtual int data_p_index(int *value);
    virtual QHash <int, QByteArray> roleNames() const;

    void updateQml();

    int add(const SettingsModel::Data &data);
    int add(const QList <SettingsModel::Data> &data);

    int insert(int *ptr_value_pos, const SettingsModel::Data &data);
    int insert(int *ptr_value_pos, const QList <SettingsModel::Data> &data);
    void insert(int pos, const QList <SettingsModel::Data> &data);

    void removeRow(int *ptr_value);
    void removeGroup(QString group);

    int countGroup(QString group);


    struct Type {
        enum {
            title,
            divider,
            combobox,
            checkbox,
            button
        };
    };

    struct Priority {
        enum {
            high,
            low
        };
    };

    struct Role {
        enum {
            type, // =Qt::UserRole + 1
            group,
            priority,
            values,
            values_data,
            value,
            name
        };
    };

private:
    QList <Data> d;

signals:
    void dataChanged(int row, int role, bool qml);
    void changed(SettingsModel *model);
};

#endif // SETTINGS_MODEL_H
