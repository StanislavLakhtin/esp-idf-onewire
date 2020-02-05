/* UART 1-wire (OneWire) Free RTOS Periherial OW_UART Driver
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ONEWIRE_UART_OW_UART_DRIVER_H
#define ONEWIRE_UART_OW_UART_DRIVER_H
#include <string.h>
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "hal/gpio_types.h"
#include "hal/uart_hal.h"

#define OW_UART_NUM     UART_NUM_1
#define OW_UART_TXD     GPIO_NUM_16
#define OW_UART_RXD     GPIO_NUM_17

#define ONEWIRE_RESET 0xF0

#define loop while (true)

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define OW_9600_BAUDRATE       9600
#define OW_115200_BAUDRATE   115200

#define OW_SIGNAL_0                    0x00 // 0x00 --default
#define OW_SIGNAL_1                    0xff
#define OW_SIGNAL_READ                 0xff

#define OW_UART_CONFIG(baudrate)             \
          {                                   \
            .baud_rate = baudrate,           \
            .data_bits = UART_DATA_8_BITS,    \
            .parity    = UART_PARITY_DISABLE, \
            .stop_bits = UART_STOP_BITS_1,    \
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,\
            .source_clk = UART_SCLK_APB,      \
          }

#define WAIT_UNTIL_DONE(what_we_wait)   \
  uint16_t _d = 0xffff;                 \
  while ( !what_we_wait && _d--)        \
    asm("nop");

#define OW_CHECK_IF_WE_SHOULD_CHANGE_BAUDRATE(baudrate)           \
  if ( ow_uart.last_baud_rate != baudrate ) {                     \
    WAIT_UNTIL_DONE( ow_uart.tx_done )                            \
    ESP_ERROR_CHECK( uart_set_baudrate( OW_UART_NUM, baudrate )); \
    ow_uart.last_baud_rate = baudrate;                            \
  }

typedef struct {
  uart_dev_t * dev;
  uint32_t rx_fifo_addr;
  uint32_t tx_fifo_addr;
  uint8_t rx;
  uint32_t last_baud_rate;
  uart_isr_handle_t * handle_ow_uart;
  bool tx_done;
} OW_UART_DEV;

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t _ow_uart_write_byte(uint32_t baudrate, uint8_t data);
uint32_t _ow_uart_read();

esp_err_t ow_uart_driver_init();
uint16_t ow_uart_reset( void );
void ow_uart_send_signal( uint16_t data );
uint16_t ow_uart_read_signal( void );

#ifdef __cplusplus
}
#endif

#endif //ONEWIRE_UART_OW_UART_DRIVER_H
