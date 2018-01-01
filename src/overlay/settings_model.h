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
