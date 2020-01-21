/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <soc/uart_reg.h>
#include <soc/uart_struct.h>
#include <esp_log.h>
#include "ow_rmt_driver.h"

OneWire ow_dev;
const char *OW_TASK_TAG = "1wire";

static void init() {
  ow_rmt_driver_init();
  ow_dev.send = ow_rmt_driver_send;
  ow_dev.reset = ow_rmt_reset;
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
