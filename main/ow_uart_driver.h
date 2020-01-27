//
// Created by Stanislav Lakhtin on 27/01/2020.
//

#ifndef ONEWIRE_UART_OW_UART_DRIVER_H
#define ONEWIRE_UART_OW_UART_DRIVER_H
#include "driver/uart.h"
#include "hal/gpio_types.h"
#include "hal/uart_hal.h"

#define OW_UART_NUM     UART_NUM_1
#define OW_UART_TXD     GPIO_NUM_16
#define OW_UART_RXD     GPIO_NUM_17
#define BUF_SIZE        50

#define loop while (true)

#define OW_UART_CONFIG(baudrate)             \
          {                                   \
            .baud_rate = baudrate,           \
            .data_bits = UART_DATA_8_BITS,    \
            .parity    = UART_PARITY_DISABLE, \
            .stop_bits = UART_STOP_BITS_1,    \
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,\
            .source_clk = UART_SCLK_APB,      \
          }

#ifdef __cplusplus
extern "C"
{
#endif


esp_err_t _ow_uart_write(uint8_t *data, uint32_t len);
uint32_t _ow_uart_read();
uint32_t _ow_uart_write_then_read(uint32_t pulse_duration_ms);

esp_err_t ow_uart_driver_init();
uint16_t ow_uart_reset( void );
void ow_uart_send_signal( uint16_t data );
uint16_t ow_uart_read_signal( void );

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_UART_DRIVER_H
