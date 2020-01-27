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

static OW_UART_DEV uart_dev = {
    .dev = UART_LL_GET_HW( OW_UART_NUM ),
    .last_baud_rate = OW_DEFAULT_BAUDRATE,
    .last_read = 0x00,
    .handle_ow_uart = NULL,
    ._baud_rate_can_change = true  // TODO change to xSemaphore to ISR
};

static void IRAM_ATTR uart_intr_handle() {
  uint32_t _fifo_len;
  switch ( uart_ll_get_intsts_mask( uart_dev.dev )) {
    case UART_INTR_TX_DONE:
      _fifo_len = uart_ll_get_rxfifo_len(uart_dev.dev);
      for (int i = 0; i < _fifo_len; i++)               // flush RX FIFO
        uart_ll_read_rxfifo( uart_dev.dev, &uart_dev.last_read, 0x01 );
      uart_ll_rxfifo_rst(uart_dev.dev);                 // reset RX FIFO
      uart_dev._baud_rate_can_change = true;
      uart_clear_intr_status(OW_UART_NUM, UART_INTR_MASK);
      break;
  }
}

esp_err_t ow_uart_driver_init() {
  uart_config_t uart_config = OW_UART_CONFIG( OW_DEFAULT_BAUDRATE );
  ESP_ERROR_CHECK( uart_param_config( OW_UART_NUM, &uart_config ));
  ESP_ERROR_CHECK( uart_set_pin( OW_UART_NUM, OW_UART_TXD, OW_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE ));
  uart_ll_ena_intr_mask( uart_dev.dev, UART_INTR_TX_DONE );
  ESP_ERROR_CHECK( uart_isr_register( OW_UART_NUM, uart_intr_handle, NULL,
                                      ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM,
                                      uart_dev.handle_ow_uart ));
  return ESP_OK;
}


esp_err_t _ow_uart_write( uint32_t baudrate, uint8_t * data, size_t len ) {
  if ( uart_dev.last_baud_rate != baudrate ) {
    while ( !uart_dev._baud_rate_can_change );
    uart_dev._baud_rate_can_change = false;
    //periph_module_disable(uart_periph_signal[OW_UART_NUM].module);
    ESP_ERROR_CHECK( uart_set_baudrate( OW_UART_NUM, baudrate ));
    uart_dev.last_baud_rate = baudrate;
    //periph_module_reset(uart_periph_signal[OW_UART_NUM].module);
    //periph_module_enable(uart_periph_signal[OW_UART_NUM].module);
  }
  uart_ll_write_txfifo( uart_dev.dev, data, len );
  return ESP_OK;
}

uint32_t _ow_uart_read() {
  return uart_dev.last_read;
}
