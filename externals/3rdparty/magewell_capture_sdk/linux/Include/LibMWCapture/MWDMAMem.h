////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing)
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////
#ifndef __MW_DMA_MEM_H__
#define __MW_DMA_MEM_H__

#pragma pack(push)
#pragma pack(1)

#include "MWSg.h"

enum mw_dma_data_direction{
    MW_DMA_BIDIRECTIONAL = 0,
    MW_DMA_TO_DEVICE = 1,
    MW_DMA_FROM_DEVICE = 2,
    MW_DMA_NONE = 3,
};

static inline int mw_valid_dma_direction(int direction)
{
    return ((direction == MW_DMA_BIDIRECTIONAL) ||
        (direction == MW_DMA_TO_DEVICE) ||
        (direction == MW_DMA_FROM_DEVICE));
}

#define MW_DMA_MEMORY_MAX_CLIENT    (16)

#define MWCAP_VIDEO_MEMORY_TYPE_USER        (3)
#define MWCAP_VIDEO_MEMORY_TYPE_PHYSICAL    (4)
#define MWCAP_VIDEO_MEMORY_TYPE_NVRDMA      (5)      /* need NvRdmaForProCapture module */
#define MWCAP_VIDEO_MEMORY_TYPE_PRIV_BASE   (6)


struct mw_dma_memory_client;

struct mw_dma_desc {
    mw_scatterlist_t            *mwsg_list;
    int                         sglen;

    /* for internal use */
    struct mw_dma_memory_client *client;
};

struct mw_dma_memory_client {
    int                     mem_type;

    int                     (*create_dma_desc)(struct mw_dma_desc **dma_desc,
                                               unsigned long addr, size_t size,
                                               int direction,
                                               void *private_data);
    int                     (*sync_for_cpu)(struct mw_dma_desc *dma_desc);
    int                     (*sync_for_device)(struct mw_dma_desc *dma_desc);
    void                    (*destroy_dma_desc)(struct mw_dma_desc *dma_desc);
};

int mw_register_dma_memory_client(struct mw_dma_memory_client *client);

int mw_unregister_dma_memory_client(struct mw_dma_memory_client *client);

#pragma pack(pop)

#endif /* __MW_DMA_MEM_H__ */
