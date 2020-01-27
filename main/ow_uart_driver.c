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

static int _baud_rate = OW_DEFAULT_BAUDRATE;
static volatile bool _baud_rate_can_change = true;
uart_isr_handle_t *handle_ow_uart = NULL;

static void IRAM_ATTR uart_intr_handle() {
  _baud_rate_can_change = true;
  uart_clear_intr_status(OW_UART_NUM, UART_INTR_MASK);
}

esp_err_t ow_uart_driver_init() {
  uart_config_t uart_config = OW_UART_CONFIG(OW_DEFAULT_BAUDRATE);
  ESP_ERROR_CHECK(uart_param_config(OW_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(OW_UART_NUM, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  uart_dev_t *uart_dev = UART_LL_GET_HW(OW_UART_NUM);
  uart_ll_ena_intr_mask(uart_dev, UART_INTR_TX_DONE);
  ESP_ERROR_CHECK(uart_isr_register(OW_UART_NUM, uart_intr_handle, NULL,
                                    ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM,
                                    handle_ow_uart));
  return ESP_OK;
}


esp_err_t _ow_uart_write(uint32_t baudrate, uint8_t *data, size_t len) {
  uart_dev_t *uart_dev = UART_LL_GET_HW(OW_UART_NUM);
  if (_baud_rate != baudrate) {
    while ( !_baud_rate_can_change );
    _baud_rate_can_change = false;
      //periph_module_disable(uart_periph_signal[OW_UART_NUM].module);
     ESP_ERROR_CHECK(uart_set_baudrate(OW_UART_NUM, baudrate));
    _baud_rate = baudrate;
    //periph_module_reset(uart_periph_signal[OW_UART_NUM].module);
    //periph_module_enable(uart_periph_signal[OW_UART_NUM].module);
  }
  uart_ll_write_txfifo(uart_dev, data, len);
  return ESP_OK;
}
