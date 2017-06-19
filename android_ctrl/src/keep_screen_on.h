#ifndef KEEP_SCREEN_ON_H
#define KEEP_SCREEN_ON_H

#include <QObject>

#if defined(Q_OS_ANDROID)

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#endif

class KeepScreenOn : public QObject
{
    Q_OBJECT

public:
    explicit KeepScreenOn(QObject *parent=0);

public slots:
    void setEnabled(bool enabled);
};

#endif // KEEP_SCREEN_ON_H
