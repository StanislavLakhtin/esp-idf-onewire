//
// Created by Stanislav Lakhtin on 16/01/2020.
//

#ifndef ONEWIRE_UART_OW_UART_DRIVER_H
#define ONEWIRE_UART_OW_UART_DRIVER_H

#include "driver/uart.h"
#include "driver/gpio.h"

#include "ow/ow.h"
#include <ow/ds18b20.h>

#define OW_UART_TXD  (GPIO_NUM_4)
#define OW_UART_RXD  (GPIO_NUM_5)
#define OW_UART UART_NUM_1

#define BUF_SIZE (256)

#define PATTERN_CHR_NUM    (3)  //todo delete
QueueHandle_t ow_uart_queue;

#ifdef __cplusplus
extern "C"
{
#endif

void ow_uart_driver_init();
void ow_uart_baudrate_setup(uint32_t baud);
void ow_uart_driver_send(char data);

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_UART_DRIVER_H
