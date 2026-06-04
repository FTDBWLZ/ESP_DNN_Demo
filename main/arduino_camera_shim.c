#include "esp_err.h"
#include "esp_private/gdma.h"

#ifdef gdma_get_channel_id
#undef gdma_get_channel_id
#endif

esp_err_t gdma_get_channel_id(gdma_channel_handle_t dma_chan, int *channel_id)
{
    return gdma_get_group_channel_id(dma_chan, NULL, channel_id);
}
