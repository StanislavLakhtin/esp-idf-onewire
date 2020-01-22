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

#define OW_RMT_TX_PIN        GPIO_NUM_16
#define OW_RMT_RX_PIN        GPIO_NUM_17
#define OW_RMT_TX_CHANNEL    RMT_CHANNEL_0
#define OW_RMT_RX_CHANNEL    RMT_CHANNEL_1

static const rmt_item32_t RESET_PULSE = { {{0x4fff, 0, 0x7fff, 1}} } ;

#ifdef __cplusplus
extern "C"
{
#endif

void ow_rmt_driver_init();

uint16_t ow_rmt_reset(void);

void ow_rmt_driver_send(uint16_t data);

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_RMT_DRIVER_H
