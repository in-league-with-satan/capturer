/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
