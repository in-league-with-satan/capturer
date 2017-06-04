#include <QDebug>

#include "vibro.h"

Vibro::Vibro(QObject *parent)
    : QObject(parent)
{
#if defined(Q_OS_ANDROID)

    QAndroidJniObject vibro_string=QAndroidJniObject::fromString("vibrator");

    QAndroidJniObject activity=QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");

    QAndroidJniObject appctx=activity.callObjectMethod("getApplicationContext","()Landroid/content/Context;");

    vibrator_service=appctx.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", vibro_string.object<jstring>());

    // vibrator_service=QAndroidJniObject::getStaticObjectField<jstring>("android/content/Context", "VIBRATOR_SERVICE");

#endif
}

void Vibro::vibrate(int milliseconds)
{
#if not defined(Q_OS_ANDROID)

    Q_UNUSED(milliseconds);

#else

    if(vibrator_service.isValid()) {
        jlong ms=milliseconds;

        jboolean hasvibro=vibrator_service.callMethod<jboolean>("hasVibrator", "()Z");

        vibrator_service.callMethod<void>("vibrate", "(J)V", ms);

    } else {
        qCritical() << "No vibrator service available";
    }

#endif
}
