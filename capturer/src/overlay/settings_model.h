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
        }

        int type;
        QStringList values;
        QVariantList values_data;
        int *value;
        QString group;
        QString name;
    };

    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(const int &index, int role) const;
    Q_INVOKABLE void setData(const int &index, int role, QVariant data, bool qml=false);
    Q_INVOKABLE void setData(int *ptr_value, int role, QVariant data, bool qml=false);

    Q_INVOKABLE int focusPrev(int index) const;
    Q_INVOKABLE int focusNext(int index) const;
    Q_INVOKABLE bool posCheck(int index) const;

    Q_INVOKABLE void reload();

    virtual SettingsModel::Data *data_p(const int &index);
    virtual SettingsModel::Data *data_p(int *value);
    virtual int data_p_index(int *value);
    virtual QHash <int, QByteArray> roleNames() const;

    int add(const SettingsModel::Data &data);

    struct Type {
        enum {
            title,
            divider,
            combobox,
            checkbox,
            button
        };
    };

    struct Role {
        enum {
            type, // =Qt::UserRole + 1
            group,
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
