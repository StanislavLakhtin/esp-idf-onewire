/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ONEWIRE_UART_OW_UART_DRIVER_H
#define ONEWIRE_UART_OW_UART_DRIVER_H
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "hal/gpio_types.h"
#include "hal/uart_hal.h"

#define OW_UART_NUM     UART_NUM_1
#define OW_UART_TXD     GPIO_NUM_16
#define OW_UART_RXD     GPIO_NUM_17
#define BUF_SIZE        12

#define LONGWAIT        1000

#define loop while (true)

#define OW_DEFAULT_BAUDRATE   9600

#define OW_UART_CONFIG(baudrate)             \
          {                                   \
            .baud_rate = baudrate,           \
            .data_bits = UART_DATA_8_BITS,    \
            .parity    = UART_PARITY_DISABLE, \
            .stop_bits = UART_STOP_BITS_1,    \
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,\
            .source_clk = UART_SCLK_APB,      \
          }

typedef struct {
  uart_dev_t * dev;
  uint8_t last_read;
  uint32_t last_baud_rate;
  uart_isr_handle_t * handle_ow_uart;
  bool _baud_rate_can_change;
} OW_UART_DEV;

#ifdef __cplusplus
extern "C"
{
#endif


esp_err_t _ow_uart_write(uint32_t baudrate, uint8_t * data, size_t len);
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
