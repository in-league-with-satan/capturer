/******************************************************************************

Copyright © 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifdef __linux__
#  include <sys/types.h>
#  include <unistd.h>
#  include <sys/syscall.h>
#endif

#include "debug_helpers.h"

bool enabled=false;

void dbgHlpSetEnabled(bool value)
{
    enabled=value;
}

int64_t getProcessId()
{
    if(!enabled)
        return 0;

#ifdef __linux__

    return syscall(SYS_gettid);

#else

    return 0;

#endif
}

void printProcessId(std::string msg)
{
    if(!enabled)
        return;

    const int64_t pid=getProcessId();

    if(pid>0) {
        qDebug().noquote() << pid << QString::fromStdString(msg);
    }
}
