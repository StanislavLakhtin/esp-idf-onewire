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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ow_uart_driver.h"

OneWire ow_dev;

const char* OW_TASK = "OW_UART";

static void init() {
  ow_uart_driver_init();
  ow_dev.usart_setup = ow_uart_baudrate_setup;
  ow_dev.send_usart = ow_uart_driver_send;
}

static void ow_task(void *arg) {
  while (1) {
    ESP_LOGI(OW_TASK, "Scan OW bus...");
    uint8_t rslt = ow_scan(&ow_dev);
    if (rslt) {
      ESP_LOGI(OW_TASK, "Found one or more devices");
      for (uint8_t i = 0; i < ow_dev.state.devicesQuantity; i++) {
        if (ow_dev.rom[i].family == 0x28) {  // Found DS18B20 Temp sensor
          ESP_LOGI(OW_TASK, "temp: %f", read_temperature(&ow_dev, &ow_dev.rom[1]));
        }
      }
    } else {
      ESP_LOGI(OW_TASK, "There are no any OW device on the bus");
    }
    for (int i = 10; i >= 0; i--) {
      ESP_LOGI(OW_TASK, "Rescaning in %d seconds...", i * 1);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

static void rx_task(void *arg) {
  static const char *RX_TASK_TAG = "RX_TASK";
  uint8_t data = 0x00;
  while (1) {
    const int rxBytes = uart_read_bytes(OW_UART, &data, 1, 10 / portTICK_RATE_MS);
    if (rxBytes > 0) {
      ow_bus_get_echo_data(&ow_dev, data);
      ESP_LOGI(RX_TASK_TAG, "Read byte: '%d'", ow_dev.state.rc_buffer);
    }
  }
}

void app_main(void) {
  init();
  xTaskCreate(ow_task, "ow_task", 2048, NULL, 10, NULL);
  xTaskCreate(rx_task, "rx_task", configMINIMAL_STACK_SIZE, NULL, 10, NULL);
}
