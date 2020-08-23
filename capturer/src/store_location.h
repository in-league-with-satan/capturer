/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef STORE_LOCATION_H
#define STORE_LOCATION_H

#include <QtGlobal>

#define store_location StoreLocation::instance()

class StoreLocation
{
public:
    static StoreLocation *createInstance();
    static StoreLocation *instance();

    QString config() const;
    QString temp() const;
    QString videos() const;

private:
    StoreLocation();

    static StoreLocation *_instance;

    bool mkPaths();

    QString loc_config;
    QString loc_temp;
    QString loc_videos;
};

#endif // STORE_LOCATION_H
