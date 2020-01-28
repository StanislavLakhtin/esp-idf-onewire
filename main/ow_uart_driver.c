/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include "ow_uart_driver.h"

static volatile OW_UART_DEV uart_dev = {
    .dev = UART_LL_GET_HW(OW_UART_NUM),
    .last_baud_rate = OW_9600_BAUDRATE,
    .rx = 0x00,
    .handle_ow_uart = NULL,
    .tx_done = true,                   // TODO change to xSemaphore to ISR ?
};

static uint8_t RX_BUFFER[BUF_SIZE];

static void IRAM_ATTR uart_intr_handle() {
  uint32_t _flags = uart_ll_get_intsts_mask(uart_dev.dev);
  if (_flags == UART_INTR_TX_DONE ) {
    uart_ll_rxfifo_rst(uart_dev.dev);                // reset RX FIFO
    uart_dev.tx_done = true;
  }
  uart_clear_intr_status(OW_UART_NUM, UART_INTR_MASK);
}

esp_err_t ow_uart_driver_init() {
  uart_config_t uart_config = OW_UART_CONFIG(uart_dev.last_baud_rate);
  ESP_ERROR_CHECK(uart_param_config(OW_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(OW_UART_NUM, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  uart_ll_ena_intr_mask(uart_dev.dev, UART_INTR_TX_DONE);
  ESP_ERROR_CHECK(uart_isr_register(OW_UART_NUM, uart_intr_handle, NULL,
                                    ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM,
                                    uart_dev.handle_ow_uart));
  memset(RX_BUFFER, 0x00, sizeof(uint8_t) * BUF_SIZE);
  return ESP_OK;
}


esp_err_t _ow_uart_write(uint32_t baudrate, uint8_t *data, size_t len) {
  OW_CHECK_IF_WE_SHOULD_CHANGE_BAUDRATE(baudrate)
  uart_dev.tx_done = false;
  uart_ll_write_txfifo(uart_dev.dev, data, len);
  return ESP_OK;
}

esp_err_t _ow_uart_write_byte(uint32_t baudrate, uint8_t data) {
  OW_CHECK_IF_WE_SHOULD_CHANGE_BAUDRATE(baudrate)
  uart_dev.tx_done = false;
  uart_ll_write_txfifo(uart_dev.dev, &data, 0x01);
  return ESP_OK;
}

inline uint32_t _ow_uart_read() {
  uint32_t _len = uart_ll_get_rxfifo_len(uart_dev.dev);
  if (_len > 0) {
    uart_ll_read_rxfifo(uart_dev.dev, RX_BUFFER, _len);
    uart_dev.rx = RX_BUFFER[_len - 1];
  } else {
    uart_dev.rx = 0x00;
  }
  return uart_dev.rx;
}

uint16_t ow_uart_reset(void) {
  _ow_uart_write_byte(OW_9600_BAUDRATE, ONEWIRE_RESET);
  WAIT_TX_DONE
  return _ow_uart_read();
}

void ow_uart_send_signal(uint16_t data) {
  uint32_t _duration_as_uart_data = data ? OW_SIGNAL_1 : OW_SIGNAL_0;
  _ow_uart_write_byte(OW_115200_BAUDRATE, _duration_as_uart_data);
}

uint16_t ow_uart_read_signal(void) {
  _ow_uart_write_byte(OW_115200_BAUDRATE, OW_SIGNAL_READ);
  WAIT_TX_DONE
  return _ow_uart_read();
}
