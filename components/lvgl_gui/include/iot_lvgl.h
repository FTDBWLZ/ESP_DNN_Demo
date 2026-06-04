#ifndef _COM_GUI_LVGL_H
#define _COM_GUI_LVGL_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#include "sdkconfig.h"

#include "lv_conf.h"
#include "lvgl.h"


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initialize LittlevGL GUI 
 */
void lvgl_init();
bool lvgl_port_lock(uint32_t timeout_ms);
void lvgl_port_unlock();

#ifdef __cplusplus
}
#endif


#endif /* _COM_GUI_LVGL_H */
