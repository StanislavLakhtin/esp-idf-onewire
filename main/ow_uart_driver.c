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

static volatile OW_UART_DEV ow_uart = {
    .dev = UART_LL_GET_HW(OW_UART_NUM),
    .last_baud_rate = OW_9600_BAUDRATE,
    .rx = 0x00,
    .handle_ow_uart = NULL,
    .tx_done = true,                   // TODO change to xSemaphore to ISR ?
};

static uint8_t RX_BUFFER[BUF_SIZE];

static void IRAM_ATTR uart_intr_handle() {
  uint32_t _flags = uart_ll_get_intsts_mask(ow_uart.dev);
  switch (_flags ) {
    case UART_INTR_TX_DONE:
      ow_uart.tx_done = true;
      break;
  }
  uart_clear_intr_status(OW_UART_NUM, UART_INTR_MASK);
}

esp_err_t ow_uart_driver_init() {
  uart_config_t uart_config = OW_UART_CONFIG(ow_uart.last_baud_rate);
  ESP_ERROR_CHECK(uart_param_config(OW_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(OW_UART_NUM, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  uart_ll_ena_intr_mask(ow_uart.dev, UART_INTR_TX_DONE );
  ESP_ERROR_CHECK(uart_isr_register(OW_UART_NUM, uart_intr_handle, NULL,
                                    ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM,
                                    ow_uart.handle_ow_uart));
  ow_uart.fifo_addr = (ow_uart.dev == &UART0) ? UART_FIFO_REG(0) : (ow_uart.dev == &UART1) ? UART_FIFO_REG(1) : UART_FIFO_REG(2);
  return ESP_OK;
}




esp_err_t _ow_uart_write(uint32_t baudrate, uint8_t *data, size_t len) {
  OW_CHECK_IF_WE_SHOULD_CHANGE_BAUDRATE(baudrate)
  ow_uart.tx_done = false;
  uart_ll_write_txfifo(ow_uart.dev, data, len);
  return ESP_OK;
}

esp_err_t _ow_uart_write_byte(uint32_t baudrate, uint8_t data) {
  OW_CHECK_IF_WE_SHOULD_CHANGE_BAUDRATE(baudrate)
  ow_uart.tx_done = false;
  uart_ll_rxfifo_rst(ow_uart.dev);
  uart_ll_write_txfifo(ow_uart.dev, &data, 0x01);
  return ESP_OK;
}

inline uint32_t _ow_uart_read() {
  uint32_t _len = uart_ll_get_rxfifo_len(ow_uart.dev);
  uart_ll_read_rxfifo(ow_uart.dev, RX_BUFFER, _len);
  ow_uart.rx = (_len > 0) ? RX_BUFFER[_len - 1] : 0x00;
  ESP_LOGI("OWDriver", "read: %c%c%c%c%c%c%c%c fifo length: %d", BYTE_TO_BINARY(ow_uart.rx), _len);
  uart_ll_rxfifo_rst(ow_uart.dev);
  return ow_uart.rx;
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
