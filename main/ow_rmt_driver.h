//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#ifndef ONEWIRE_UART_OW_RMT_DRIVER_H
#define ONEWIRE_UART_OW_RMT_DRIVER_H

#include "ow/ow.h"
#include <ow/ds18b20.h>

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
