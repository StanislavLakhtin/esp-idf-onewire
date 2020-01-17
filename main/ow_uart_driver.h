//
// Created by Stanislav Lakhtin on 16/01/2020.
//

#ifndef ONEWIRE_UART_OW_UART_DRIVER_H
#define ONEWIRE_UART_OW_UART_DRIVER_H

#include "driver/uart.h"
#include "driver/gpio.h"

#include "ow/ow.h"

#ifdef __cplusplus
extern "C"
{
#endif

void ow_uart_driver_setup(uint32_t baud);
void ow_uart_driver_send(char data);

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_UART_DRIVER_H
