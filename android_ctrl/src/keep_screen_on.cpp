#if defined(Q_OS_ANDROID)

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#endif

#include "keep_screen_on.h"


const int FLAG_KEEP_SCREEN_ON=128;


KeepScreenOn::KeepScreenOn(QObject *parent)
    : QObject(parent)
{
}

void KeepScreenOn::setEnabled(bool enabled)
{
#if !defined(Q_OS_ANDROID)

    Q_UNUSED(enabled)

#else

    QAndroidJniObject activity=QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");

    if(activity.isValid()) {
        QAndroidJniObject window=activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

        if(window.isValid()) {
            if(enabled)
                window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);

            else
                window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        }

        QAndroidJniEnvironment env;

        if(env->ExceptionCheck())
            env->ExceptionClear();
    }

#endif
}
