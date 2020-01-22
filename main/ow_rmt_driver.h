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

#define OW_RMT_RX_BUFFER_SIZE   8

#define APB_CLK                 80000000    // todo пересчитать все tick от реального значения APB_CLK
#define CLK_DIV                 10

#define _RESET_DURATION         480
#define _RESET_IDLE_DURATION    70
#define _IDLE_DURATION          70

#define OW_MS_TO_RMT_TICKS( ms )  ( ms / 80 * CLK_DIV)

#define OW_TICK_SLOT_MS_LOW_HIGH( low_duration, high_duration )   \
    {                                                             \
        {                                                         \
            .duration0 = OW_MS_TO_RMT_TICKS(low_duration),        \
            .level0 = 0,                                          \
            .duration1 = OW_MS_TO_RMT_TICKS(high_duration),       \
            .level1 = 1                                           \
        }                                                         \
    }

/*
 * 1ms это слишком мало [как строительная единица] для формирования нормального сигнала 1-wire.
 * Более менее нормальная единица измерения это 8ms. Руководствуясь этим соображением, размер слота
 * для чтения составляет 8ms .
 * Типовая частота APB_CLK = 80 000 000 Hz
 * 1 ms достигается при стандартном делителе в 80
 * соответственно, 8 ms мы можем достичь с делителем в 10
 * Чтобы обеспечить нужное количество tick, делитель (clk_div) на RX канале мы определяем
 * в 10 (_config_rx.clk_div = 10;)
 * размер Tick : 1 сек / ( 80 000 000 / 10 )
 * Макрос OW_TICK_SLOT_MS_LOW_HIGH нужен, чтобы удобно определять структуру в одну строчку, делая код плотнее
 * */

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
