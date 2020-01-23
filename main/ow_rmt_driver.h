//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#ifndef ONEWIRE_UART_OW_RMT_DRIVER_H
#define ONEWIRE_UART_OW_RMT_DRIVER_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "ow/ow.h"
#include <ow/ds18b20.h>

#define OW_RMT_TX_PIN           GPIO_NUM_16
#define OW_RMT_RX_PIN           GPIO_NUM_17
#define OW_RMT_TX_CHANNEL       RMT_CHANNEL_0
#define OW_RMT_RX_CHANNEL       RMT_CHANNEL_1

#define OW_RMT_RX_BUFFER_SIZE   28          // elements (CLK_DIV = 200, 70ms = 28 elements)

#define APB_CLK                 80000000    // todo пересчитать все tick от реального значения APB_CLK
#define CLK_DIV                 240

#define _RMT_OVERDRIVE          2
#define _RESET_DURATION         480
#define _RESET_IDLE_DURATION    70
#define _IDLE_DURATION          70

#define OW_MS_TO_RMT_TICKS( ms )  ( ( ( ms ) * 10 ) / ( CLK_DIV / 8 )  )

#define OW_TICK_SLOT_MS_LOW( low_duration_ms )   \
    {                                                             \
        {                                                         \
            .duration0 = OW_MS_TO_RMT_TICKS(low_duration_ms),     \
            .level0 = 0,                                          \
            .duration1 = 0,                                       \
            .level1 = 1                                           \
        }                                                         \
    }

#ifdef __cplusplus
extern "C"
{
#endif

void ow_rmt_driver_init();

uint16_t ow_rmt_reset( void );

void ow_rmt_driver_send( uint16_t data );

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_RMT_DRIVER_H
