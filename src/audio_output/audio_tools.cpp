#include "audio_tools.h"

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst)
{
    ba_dst->resize(ba_src->size()/8*2);

    uint32_t *ptr_data_src=(uint32_t*)ba_src->data();
    uint32_t *ptr_data_dst=(uint32_t*)ba_dst->data();

    for(int pos_src=0, pos_dst=0, size=ba_src->size()/4; pos_src<size; pos_src+=4)
        ptr_data_dst[pos_dst++]=ptr_data_src[pos_src];
}

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst)
{
    ba_dst->resize(ba_src->size()/8*6);
    ba_dst->fill(0);

    uint16_t *ptr_data_src=(uint16_t*)ba_src->data();
    uint16_t *ptr_data_dst=(uint16_t*)ba_dst->data();

    for(int pos_src=0, pos_dst=0, size=ba_src->size()/8; pos_src<size; pos_src+=8, pos_dst+=6) {
        ptr_data_dst[pos_dst + 0]=ptr_data_src[pos_src + 0];    // front left;
        ptr_data_dst[pos_dst + 1]=ptr_data_src[pos_src + 1];    // front right;
        ptr_data_dst[pos_dst + 2]=ptr_data_src[pos_src + 2];    // lfe;
        ptr_data_dst[pos_dst + 3]=ptr_data_src[pos_src + 3];    // front center;
        ptr_data_dst[pos_dst + 4]=ptr_data_src[pos_src + 6];    // back left;
        ptr_data_dst[pos_dst + 5]=ptr_data_src[pos_src + 7];    // back right;
    }
}
