#ifndef PROTECT_H
#define PROTECT_H

#include <QMutex>
#include <QMutexLocker>

template <class T>
class Protect
{
public:
    operator T() {
        QMutexLocker ml(&mutex);
        return value;
    }

    void operator=(const T &right) {
        QMutexLocker ml(&mutex);
        this->value=right;
    }

private:
    T value;
    QMutex mutex;
};

#endif // PROTECT_H
