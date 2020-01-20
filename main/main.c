/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <ow/ds18b20.h>
#include <esp_log.h>
#include <soc/uart_reg.h>
#include <soc/uart_struct.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ow_uart_driver.h"

OneWire ow_dev;
const char* OW_TASK_TAG = "1wire";
static intr_handle_t ow_uart_handle;

static void IRAM_ATTR ow_rx_isr(void* arg) {
  volatile uart_dev_t *uart = &UART1;
  ESP_LOGI(OW_TASK_TAG, "rx isr routine");
  while (uart->status.rxfifo_cnt) {
    uint8_t _d = uart->fifo.rw_byte;
    ow_bus_get_echo_data(&ow_dev, _d);
    ESP_LOGI(OW_TASK_TAG, "get %x", _d);
  }
  uart_clear_intr_status(UART_NUM_0, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
}

static void init() {
  ow_uart_driver_init();
  ow_dev.usart_setup = ow_uart_baudrate_setup;
  ow_dev.send_usart = ow_uart_driver_send;
  uart_isr_register(OW_UART, ow_rx_isr, NULL, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM, &ow_uart_handle);
  uart_enable_rx_intr(OW_UART);
}

static void ow_task(void *arg) {
  while (1) {
    ESP_LOGI(OW_TASK_TAG, "Scan OW bus...");
    uint8_t rslt = ow_scan(&ow_dev);
    if (rslt) {
      ESP_LOGI(OW_TASK_TAG, "Found one or more devices");
      for (uint8_t i = 0; i < ow_dev.state.devicesQuantity; i++) {
        if (ow_dev.rom[i].family == 0x28) {  // Found DS18B20 Temp sensor
          ESP_LOGI(OW_TASK_TAG, "temp: %f", read_temperature(&ow_dev, &ow_dev.rom[1]));
        }
      }
    } else {
      ESP_LOGI(OW_TASK_TAG, "There are no any OW device on the bus");
    }
    for (int i = 10; i >= 0; i--) {
      ESP_LOGI(OW_TASK_TAG, "Rescaning in %d seconds...", i * 1);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}



void app_main(void) {
  init();
  xTaskCreate(ow_task, "ow_task", 2048, NULL, 10, NULL);
}
