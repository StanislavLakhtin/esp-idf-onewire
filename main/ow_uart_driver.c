//
// Created by Stanislav Lakhtin on 27/01/2020.
//

#include "ow_uart_driver.h"

esp_err_t ow_uart_driver_init() {
  uart_config_t uart_config = OW_UART_CONFIG(OW_DEFAULT_BAUDRATE);
  //ESP_ERROR_CHECK(uart_driver_install(OW_UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(OW_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(OW_UART_NUM, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  return ESP_OK;
}

static int _baud_rate = OW_DEFAULT_BAUDRATE;

esp_err_t _ow_uart_write(uint32_t baudrate, uint8_t *data, uint32_t len) {
  if (_baud_rate != baudrate) {
    periph_module_disable(uart_periph_signal[OW_UART_NUM].module);
    ESP_ERROR_CHECK(uart_set_baudrate(OW_UART_NUM, baudrate));
    _baud_rate = baudrate;
    periph_module_enable(uart_periph_signal[OW_UART_NUM].module);
  }
  uart_dev_t * uart_dev = UART_LL_GET_HW(OW_UART_NUM);
  uart_ll_write_txfifo(uart_dev, data, len);
  uart_ll_write_txfifo(uart_dev, data, len);
  return ESP_OK;
}
