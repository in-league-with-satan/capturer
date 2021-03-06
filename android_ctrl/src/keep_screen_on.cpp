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
