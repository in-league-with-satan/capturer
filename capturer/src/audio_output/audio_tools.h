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

#ifndef AUDIO_TOOLS_H
#define AUDIO_TOOLS_H

#include <QByteArray>

template <typename T>
void channelsRemap(void *data, size_t size)
{
    T *ptr_data=(T*)data;

    for(int pos=0, csize=size/sizeof(T); pos<csize; pos+=8) {
        // swap center and lfe
        std::swap(ptr_data[pos + 2], ptr_data[pos + 3]);

        // swap side left and rear left
        std::swap(ptr_data[pos + 4], ptr_data[pos + 6]);

        // swap side right and rear right
        std::swap(ptr_data[pos + 5], ptr_data[pos + 7]);
    }
}

template <typename T>
void copyAudioSamplesMagewell(void *ptr_src, void *ptr_dst, int samples_per_frame, int channels)
{
    // front left = 0
    // front right = 4
    // center = 5
    // lfe = 1
    // back left = 3
    // back right = 7
    // side left = 2
    // side right = 6

    const static int ch_mapping[8]={
        0, 4, 5, 1, 3, 7, 2, 6
    };

    const static int ch_mapping_6_wide[8]={
        0, 4, 5, 1, 2, 6, 3, 7,
    };

    // const static int ch_mapping_4[8]={
    //     0, 4, 2, 6, 5, 1, 3, 7,
    // };

    uint32_t *ptr_r=(uint32_t*)ptr_src;
    T *ptr_w=(T*)ptr_dst;

    const int sample_size_bytes=sizeof(T);

    if(sample_size_bytes==2) {
        if(channels==6) {
            for(int smpl_num=0; smpl_num<samples_per_frame; smpl_num++) {
                for(int ch_num=0; ch_num<channels; ch_num+=2) {
                    ptr_w[smpl_num*channels + ch_num + 0]=ptr_r[smpl_num*8 + ch_mapping_6_wide[ch_num + 0]] >> 16;
                    ptr_w[smpl_num*channels + ch_num + 1]=ptr_r[smpl_num*8 + ch_mapping_6_wide[ch_num + 1]] >> 16;
                }
            }

        } else {
            for(int smpl_num=0; smpl_num<samples_per_frame; smpl_num++) {
                for(int ch_num=0; ch_num<channels; ch_num+=2) {
                    ptr_w[smpl_num*channels + ch_num + 0]=ptr_r[smpl_num*8 + ch_mapping[ch_num + 0]] >> 16;
                    ptr_w[smpl_num*channels + ch_num + 1]=ptr_r[smpl_num*8 + ch_mapping[ch_num + 1]] >> 16;
                }
            }
        }

    } else {
        if(channels==6) {
            for(int smpl_num=0; smpl_num<samples_per_frame; smpl_num++) {
                for(int ch_num=0; ch_num<channels; ch_num+=2) {
                    ptr_w[smpl_num*channels + ch_num + 0]=ptr_r[smpl_num*8 + ch_mapping_6_wide[ch_num + 0]];
                    ptr_w[smpl_num*channels + ch_num + 1]=ptr_r[smpl_num*8 + ch_mapping_6_wide[ch_num + 1]];
                }
            }

        } else {
            for(int smpl_num=0; smpl_num<samples_per_frame; smpl_num++) {
                for(int ch_num=0; ch_num<channels; ch_num+=2) {
                    ptr_w[smpl_num*channels + ch_num + 0]=ptr_r[smpl_num*8 + ch_mapping[ch_num + 0]];
                    ptr_w[smpl_num*channels + ch_num + 1]=ptr_r[smpl_num*8 + ch_mapping[ch_num + 1]];
                }
            }
        }
    }
}

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst);

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst);

#endif // AUDIO_TOOLS_H
