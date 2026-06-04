#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

void vPortEvaluateYieldFromISR(int argc, ...)
{
    if (argc <= 0) {
        portYIELD_FROM_ISR();
        return;
    }

    va_list args;
    va_start(args, argc);
    BaseType_t xHigherPriorityTaskWoken = (BaseType_t)va_arg(args, int);
    va_end(args);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
