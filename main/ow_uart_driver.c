/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "ow_uart_driver.h"

static int _created_uart_conf = 0x00;

void ow_uart_driver_init() {
  if (_created_uart_conf) {
    uart_driver_delete(OW_UART);
    _created_uart_conf = 0x00;
  }
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  uart_driver_install(OW_UART, BUF_SIZE *2 , BUF_SIZE , 0, NULL, 0);
  uart_param_config(OW_UART, &uart_config);
  uart_set_pin(OW_UART, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  _created_uart_conf = 0x01;
}

void ow_uart_baudrate_setup(uint32_t baud) {
  uart_set_baudrate(OW_UART, baud);
}

void ow_uart_driver_send(const char data) {
  uart_write_bytes(OW_UART, &data, 0x01);
}
