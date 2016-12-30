#ifndef AUDIO_TOOLS_H
#define AUDIO_TOOLS_H

#include <QByteArray>


void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst);

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst);


#endif // AUDIO_TOOLS_H
