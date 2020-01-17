/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "ow_uart_driver.h"

/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define OW_UART_TXD  (GPIO_NUM_4)
#define OW_UART_RXD  (GPIO_NUM_5)
#define OW_UART UART_NUM_1

#define BUF_SIZE (256)

static int _created_uart_conf = 0x00;

void ow_uart_driver_setup(uint32_t baud) {
  if (_created_uart_conf) {
    uart_driver_delete(OW_UART);
    _created_uart_conf = 0x00;
  }
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = baud,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  uart_driver_install(OW_UART, BUF_SIZE *2 , BUF_SIZE , 0, NULL, 0);
  uart_param_config(OW_UART, &uart_config);
  uart_disable_tx_intr(OW_UART);
  uart_set_pin(UART_NUM_1, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  _created_uart_conf = 0x01;
}

void ow_uart_driver_send(const char data) {
  uart_write_bytes(OW_UART, &data, 0x01);
  //uart_wait_tx_done(OW_UART, 100);
}