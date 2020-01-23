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

#define OW_RMT_RX_BUFFER_SIZE   100         //

#define APB_CLK                 80000000    // todo пересчитать все tick от реального значения APB_CLK
#define CLK_DIV                 240

#define _RESET_DURATION         490
#define _PRESENCE_LOWER_BORDER  60
#define _PRESENCE_HIGH_BORDER   240
#define _THRESHOLD_DURATION     70
#define _WRITE_1_DURATION       7
#define _WRITE_0_DURATION       70
#define _READ_DURATION          _WRITE_1_DURATION

#define OW_MS_TO_RMT_TICKS( ms )  ( ( ( ms ) * 10 ) / ( CLK_DIV / 8 )  )
#define OW_TICKS_TO_MS( ms )      ( ms * CLK_DIV / 80 )

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


esp_err_t _ow_rmt_write(uint32_t pulse_duration_ms);
uint32_t _ow_rmt_read();
uint32_t _ow_rmt_write_then_read(uint32_t pulse_duration_ms);

esp_err_t ow_rmt_driver_init();
uint16_t ow_rmt_reset( void );
void ow_rmt_send_signal( uint16_t data );
uint16_t ow_rmt_read_signal( void );

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_RMT_DRIVER_H
