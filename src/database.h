#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>

struct Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent=0);
    ~Database();

    bool addSnapshot(QString key, const QByteArray &data);
    bool snapshot(QString key, QByteArray *data);
    bool removeSnapshot(QString key);
    void dropOldSnapshots();

private:
    void tableInit();

    QString dbname;
};

#endif // DATABASE_H
