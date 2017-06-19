#ifndef VIBRO_H
#define VIBRO_H

#include <QObject>

#if defined(Q_OS_ANDROID)

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#endif

class Vibro : public QObject
{
    Q_OBJECT

public:
    explicit Vibro(QObject *parent=0);

public slots:
    void vibrate(int milliseconds);

private:
#if defined(Q_OS_ANDROID)

    QAndroidJniObject vibrator_service;

#endif

};

#endif // VIBRO_H
