//
// Created by Stanislav Lakhtin on 27/01/2020.
//

#include "ow_uart_driver.h"

static int _baud_rate = OW_DEFAULT_BAUDRATE;
static SemaphoreHandle_t xOWWrite = NULL;

esp_err_t ow_uart_driver_init() {
  uart_config_t uart_config = OW_UART_CONFIG(OW_DEFAULT_BAUDRATE);
  //ESP_ERROR_CHECK(uart_driver_install(OW_UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(OW_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(OW_UART_NUM, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  xOWWrite = xSemaphoreCreateBinary();
  return ESP_OK;
}


esp_err_t _ow_uart_write(uint32_t baudrate, uint8_t *data, size_t len) {
  if (_baud_rate != baudrate) {
    //periph_module_disable(uart_periph_signal[OW_UART_NUM].module);
    ESP_ERROR_CHECK(uart_set_baudrate(OW_UART_NUM, baudrate));
    _baud_rate = baudrate;
    //periph_module_reset(uart_periph_signal[OW_UART_NUM].module);
    //periph_module_enable(uart_periph_signal[OW_UART_NUM].module);
  }
  uart_dev_t *uart_dev = UART_LL_GET_HW(OW_UART_NUM);
  uart_ll_write_txfifo(uart_dev, data, len);
  xSemaphoreGive(xOWWrite);
  return ESP_OK;
}
