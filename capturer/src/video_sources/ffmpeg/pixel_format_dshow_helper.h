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

#ifndef PIXEL_FORMAT_DSHOW_HELPER_H
#define PIXEL_FORMAT_DSHOW_HELPER_H

#ifdef __WIN32__

#include <windows.h>
#include <dshow.h>

GUID toDshowPixelFormat(uint32_t pix_fmt);
uint32_t fromDshowPixelFormat(const GUID &value);

#endif

#endif // PIXEL_FORMAT_DSHOW_HELPER_H
