#ifndef AUDIO_TOOLS_H
#define AUDIO_TOOLS_H

#include <QByteArray>

void channelsRemap16(QByteArray *ba_data);
void channelsRemap32(QByteArray *ba_data);

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst);

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst);


#endif // AUDIO_TOOLS_H
