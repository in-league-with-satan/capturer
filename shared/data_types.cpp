/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include "data_types.h"


QString KeyCodeC::key_title[KeyCodeC::enm_size]={
    QStringLiteral("Up"),
    QStringLiteral("Down"),
    QStringLiteral("Left"),
    QStringLiteral("Right"),
    QStringLiteral("Enter"),
    QStringLiteral("Menu"),
    QStringLiteral("Back"),
    QStringLiteral("About"),
    QStringLiteral("Info"),
    QStringLiteral("Preview Primary"),
    QStringLiteral("Preview Secondary"),
    QStringLiteral("Preview Secondary change position"),
    QStringLiteral("Preview switch half-fps"),
    QStringLiteral("HDR to SDR shader"),
    QStringLiteral("HDR shader brightness +"),
    QStringLiteral("HDR shader brightness -"),
    QStringLiteral("HDR shader saturation +"),
    QStringLiteral("HDR shader saturation -"),
    QStringLiteral("Full Screen"),
    QStringLiteral("File Browser"),
    QStringLiteral("Rec"),
    QStringLiteral("Rec State"),
    QStringLiteral("Exit")
};


QString KeyCodeC::toString(int code)
{
    if(code<0 || code>=enm_size)
        return QString();

    return key_title[code];
}

int KeyCodeC::fromString(const QString &str)
{
    for(int i=0; i<enm_size; ++i)
        if(QString::compare(str, key_title[i], Qt::CaseInsensitive)==0)
            return i;

    return Exit;
}

void KeyCodeC::declareQML()
{
    qmlRegisterType<KeyCodeC>("FuckTheSystem", 0, 0, "KeyCode");
}

//

NRecStats::NRecStats(QTime time, double avg_bitrate, quint64 size, quint32 dropped_frames_counter, quint16 frame_buffer_size, quint16 frame_buffer_used)
{
    this->time=time;
    this->avg_bitrate=avg_bitrate;
    this->size=size;
    this->dropped_frames_counter=dropped_frames_counter;
    this->frame_buffer_size=frame_buffer_size;
    this->frame_buffer_used=frame_buffer_used;
}

bool NRecStats::isNull() const
{
    return time.isNull();
}

QVariantMap NRecStats::toExt()
{
    QVariantMap map_root;

    map_root.insert(QStringLiteral("time"), time);
    map_root.insert(QStringLiteral("avg_bitrate"), avg_bitrate);
    map_root.insert(QStringLiteral("size"), size);
    map_root.insert(QStringLiteral("dropped_frames_counter"), dropped_frames_counter);
    map_root.insert(QStringLiteral("frame_buffer_size"), frame_buffer_size);
    map_root.insert(QStringLiteral("frame_buffer_used"), frame_buffer_used);

    return map_root;
}

NRecStats &NRecStats::fromExt(const QVariantMap &map_root)
{
    time=map_root.value(QStringLiteral("time")).toTime();
    avg_bitrate=map_root.value(QStringLiteral("avg_bitrate")).toDouble();
    size=map_root.value(QStringLiteral("size")).toULongLong();
    dropped_frames_counter=map_root.value(QStringLiteral("dropped_frames_counter")).toUInt();
    frame_buffer_size=map_root.value(QStringLiteral("frame_buffer_size")).toUInt();
    frame_buffer_used=map_root.value(QStringLiteral("frame_buffer_used")).toUInt();

    return *this;
}

//

PlayerState::PlayerState()
{
    duration=0;
    position=0;
}

QVariantMap PlayerState::toExt()
{
    QVariantMap map_root;

    map_root.insert(QStringLiteral("duration"), duration);
    map_root.insert(QStringLiteral("position"), position);

    return map_root;
}

PlayerState &PlayerState::fromExt(const QVariantMap &map_root)
{
    duration=map_root.value(QStringLiteral("duration")).toLongLong();
    position=map_root.value(QStringLiteral("position")).toLongLong();

    return *this;
}

//

NvState::NvState()
{
    temperature=0;
    graphic_processing_unit=0;
    memory_controller_unit=0;
    video_processing_unit=0;
}

QVariantMap NvState::toExt() const
{
    QVariantMap map_root;

    map_root.insert(QStringLiteral("dev_name"), dev_name);
    map_root.insert(QStringLiteral("temperature"), temperature);
    map_root.insert(QStringLiteral("graphic_processing_unit"), graphic_processing_unit);
    map_root.insert(QStringLiteral("memory_controller_unit"), memory_controller_unit);
    map_root.insert(QStringLiteral("video_processing_unit"), video_processing_unit);

    return map_root;
}

NvState &NvState::fromExt(const QVariantMap &map_root)
{
    dev_name=map_root.value(QStringLiteral("dev_name")).toString();
    temperature=map_root.value(QStringLiteral("temperature")).toInt();
    graphic_processing_unit=map_root.value(QStringLiteral("graphic_processing_unit")).toInt();
    memory_controller_unit=map_root.value(QStringLiteral("memory_controller_unit")).toInt();
    video_processing_unit=map_root.value(QStringLiteral("video_processing_unit")).toInt();

    return *this;
}

//

Status::Status()
{
    free_space=0;
}

QVariantMap Status::toExt()
{
    QVariantMap map_root;

    map_root.insert(QStringLiteral("input_format"), input_format);
    map_root.insert(QStringLiteral("rec_stats"), rec_stats.toExt());
    map_root.insert(QStringLiteral("player_state"), player_state.toExt());
    map_root.insert(QStringLiteral("nv_state"), nv_state.toExt());
    map_root.insert(QStringLiteral("free_space"), free_space);

    return map_root;
}

Status &Status::fromExt(const QVariantMap &map_root)
{
    input_format=map_root.value(QStringLiteral("input_format")).toString();
    rec_stats.fromExt(map_root.value(QStringLiteral("rec_stats")).toMap());
    player_state.fromExt(map_root.value(QStringLiteral("player_state")).toMap());
    nv_state.fromExt(map_root.value(QStringLiteral("nv_state")).toMap());
    free_space=map_root.value(QStringLiteral("free_space"), 0).toULongLong();
    return *this;
}

QVariantMap InputFormat::toExt()
{
    QVariantMap map_root;

    map_root.insert(QStringLiteral("width"), width);
    map_root.insert(QStringLiteral("height"), height);
    map_root.insert(QStringLiteral("frame_duration"), frame_duration);
    map_root.insert(QStringLiteral("frame_scale"), frame_scale);
    map_root.insert(QStringLiteral("progressive_frame"), progressive_frame);
    map_root.insert(QStringLiteral("pixel_format"), pixel_format);

    return map_root;
}

InputFormat &InputFormat::fromExt(const QVariantMap &map_root)
{
    width=map_root.value(QStringLiteral("width")).toInt();
    height=map_root.value(QStringLiteral("height")).toInt();
    frame_duration=map_root.value(QStringLiteral("frame_duration")).toULongLong();
    frame_scale=map_root.value(QStringLiteral("frame_scale")).toULongLong();
    progressive_frame=map_root.value(QStringLiteral("progressive_frame")).toBool();
    pixel_format=map_root.value(QStringLiteral("pixel_format")).toString();

    return *this;
}
