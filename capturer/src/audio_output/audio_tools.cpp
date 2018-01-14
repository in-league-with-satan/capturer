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

#include <QDebug>

#include "audio_tools.h"


void channelsRemap16(void *data, size_t size)
{
    int16_t *ptr_data=(int16_t*)data;

    int16_t tmp;

    for(int pos=0, csize=size/2; pos<csize; pos+=8) {
        // swap center and lfe
        tmp=ptr_data[pos + 2];
        ptr_data[pos + 2]=ptr_data[pos + 3];
        ptr_data[pos + 3]=tmp;

        // swap side left and rear left
        tmp=ptr_data[pos + 4];
        ptr_data[pos + 4]=ptr_data[pos + 6];
        ptr_data[pos + 6]=tmp;

        // swap side right and rear right
        tmp=ptr_data[pos + 5];
        ptr_data[pos + 5]=ptr_data[pos + 7];
        ptr_data[pos + 7]=tmp;
    }
}

void channelsRemap32(void *data, size_t size)
{
    int32_t *ptr_data=(int32_t*)data;

    int32_t tmp;

    for(int pos=0, csize=size/4; pos<csize; pos+=8) {
        // swap center and lfe
        tmp=ptr_data[pos + 2];
        ptr_data[pos + 2]=ptr_data[pos + 3];
        ptr_data[pos + 3]=tmp;

        // swap side left and rear left
        tmp=ptr_data[pos + 4];
        ptr_data[pos + 4]=ptr_data[pos + 6];
        ptr_data[pos + 6]=tmp;

        // swap side right and rear right
        tmp=ptr_data[pos + 5];
        ptr_data[pos + 5]=ptr_data[pos + 7];
        ptr_data[pos + 7]=tmp;
    }
}

void channelsRemap16(QByteArray *ba_data)
{
    channelsRemap16((void*)ba_data->constData(), ba_data->size());
}

void channelsRemap32(QByteArray *ba_data)
{
    channelsRemap32((void*)ba_data->constData(), ba_data->size());
}

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst)
{
    ba_dst->resize(ba_src->size()/8*2);

    // uint32_t *ptr_data_src=(uint32_t*)ba_src->constData();
    // uint32_t *ptr_data_dst=(uint32_t*)ba_dst->constData();

    // for(int pos_src=0, pos_dst=0, size=ba_src->size()/4; pos_src<size; pos_src+=4)
    //     ptr_data_dst[pos_dst++]=ptr_data_src[pos_src];

    int16_t *ptr_data_src=(int16_t*)ba_src->constData();
    int16_t *ptr_data_dst=(int16_t*)ba_dst->constData();

    for(int pos_src=0, pos_dst=0, size=ba_src->size()/2; pos_src<size; pos_src+=8, pos_dst+=2) {
        //                                  fr l,r                               fr c                         side l,r                        rear l,r
        ptr_data_dst[pos_dst + 0]=ptr_data_src[pos_src + 0]*.25 + ptr_data_src[pos_src + 2]*.25 + ptr_data_src[pos_src + 4]*.25 + ptr_data_src[pos_src + 6]*.25;
        ptr_data_dst[pos_dst + 1]=ptr_data_src[pos_src + 1]*.25 + ptr_data_src[pos_src + 2]*.25 + ptr_data_src[pos_src + 5]*.25 + ptr_data_src[pos_src + 7]*.25;
    }
}

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst)
{
    ba_dst->resize(ba_src->size()/8*6);
    ba_dst->fill(0);

    int16_t *ptr_data_src=(int16_t*)ba_src->constData();
    int16_t *ptr_data_dst=(int16_t*)ba_dst->constData();

    for(int pos_src=0, pos_dst=0, size=ba_src->size()/2; pos_src<size; pos_src+=8, pos_dst+=6) {
        // ptr_data_dst[pos_dst + 0]=ptr_data_src[pos_src + 0];    // front left;
        // ptr_data_dst[pos_dst + 1]=ptr_data_src[pos_src + 1];    // front right;
        // ptr_data_dst[pos_dst + 2]=ptr_data_src[pos_src + 2];    // front center;
        // ptr_data_dst[pos_dst + 3]=ptr_data_src[pos_src + 3];    // lfe;
        // ptr_data_dst[pos_dst + 4]=ptr_data_src[pos_src + 4];    // rear left;
        // ptr_data_dst[pos_dst + 5]=ptr_data_src[pos_src + 5];    // rear right;

        ptr_data_dst[pos_dst + 0]=ptr_data_src[pos_src + 0]*.6 + ptr_data_src[pos_src + 6]*.4;    // front left;
        ptr_data_dst[pos_dst + 1]=ptr_data_src[pos_src + 1]*.6 + ptr_data_src[pos_src + 7]*.4;    // front right;
        ptr_data_dst[pos_dst + 2]=ptr_data_src[pos_src + 2]*.6;                                   // front center;
        ptr_data_dst[pos_dst + 3]=ptr_data_src[pos_src + 3]*.6;                                   // lfe;
        ptr_data_dst[pos_dst + 4]=ptr_data_src[pos_src + 4]*.6 + ptr_data_src[pos_src + 6]*.4;    // back left;
        ptr_data_dst[pos_dst + 5]=ptr_data_src[pos_src + 5]*.6 + ptr_data_src[pos_src + 7]*.4;    // back right;
    }
}

