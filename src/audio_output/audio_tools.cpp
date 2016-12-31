#include <QDebug>

#include "audio_tools.h"

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst)
{
    ba_dst->resize(ba_src->size()/8*2);

    // uint32_t *ptr_data_src=(uint32_t*)ba_src->data();
    // uint32_t *ptr_data_dst=(uint32_t*)ba_dst->data();

    // for(int pos_src=0, pos_dst=0, size=ba_src->size()/4; pos_src<size; pos_src+=4)
    //     ptr_data_dst[pos_dst++]=ptr_data_src[pos_src];

    int16_t *ptr_data_src=(int16_t*)ba_src->data();
    int16_t *ptr_data_dst=(int16_t*)ba_dst->data();

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

    int16_t *ptr_data_src=(int16_t*)ba_src->data();
    int16_t *ptr_data_dst=(int16_t*)ba_dst->data();

    for(int pos_src=0, pos_dst=0, size=ba_src->size()/2; pos_src<size; pos_src+=8, pos_dst+=6) {
        // ptr_data_dst[pos_dst + 0]=ptr_data_src[pos_src + 0];    // front left;
        // ptr_data_dst[pos_dst + 1]=ptr_data_src[pos_src + 1];    // front right;
        // ptr_data_dst[pos_dst + 2]=ptr_data_src[pos_src + 2];    // front center;
        // ptr_data_dst[pos_dst + 3]=ptr_data_src[pos_src + 3];    // lfe;
        // ptr_data_dst[pos_dst + 4]=ptr_data_src[pos_src + 6];    // rear left;
        // ptr_data_dst[pos_dst + 5]=ptr_data_src[pos_src + 7];    // rear right;

        ptr_data_dst[pos_dst + 0]=ptr_data_src[pos_src + 0]*.6 + ptr_data_src[pos_src + 4]*.4;    // front left;
        ptr_data_dst[pos_dst + 1]=ptr_data_src[pos_src + 1]*.6 + ptr_data_src[pos_src + 5]*.4;    // front right;
        ptr_data_dst[pos_dst + 2]=ptr_data_src[pos_src + 2]*.6;                                   // front center;
        ptr_data_dst[pos_dst + 3]=ptr_data_src[pos_src + 3]*.6;                                   // lfe;
        ptr_data_dst[pos_dst + 4]=ptr_data_src[pos_src + 6]*.6 + ptr_data_src[pos_src + 4]*.4;    // back left;
        ptr_data_dst[pos_dst + 5]=ptr_data_src[pos_src + 7]*.6 + ptr_data_src[pos_src + 5]*.4;    // back right;
    }
}
