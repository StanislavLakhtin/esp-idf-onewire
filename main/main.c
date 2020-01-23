/* UART Echo Example
 *
 * Created by Stanislav Lakhtin on 06.01.2020.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "ow_rmt_driver.h"

OneWire ow_dev;
const char *OW_TASK_TAG = "1wire";

static void init() {
  ow_rmt_driver_init();
  //ow_dev.send = ow_rmt_driver_send;
  ow_dev.reset = ow_rmt_reset;
  ow_dev.write = ow_rmt_send_signal;
  ow_dev.read = ow_rmt_read_signal;
}

static void test_task(void *arg) {
  while (1) {
    uint16_t _presence = ow_dev.reset();
    if (_presence > 0) {
      ESP_LOGI(OW_TASK_TAG, "PRESENCE detected on 1-wire bus. Duration: %d ms", _presence);
      ESP_LOGI(OW_TASK_TAG, "Scan OW bus...");
      if (ow_scan(&ow_dev)) {
        for (uint8_t i = 0; i < ow_dev.state.devicesQuantity; i++) {
          if (ow_dev.rom[i].family == 0x28) {  // Found DS18B20 Temp sensor
            float _temp = read_temperature(&ow_dev, &ow_dev.rom[i]);
            ESP_LOGI(OW_TASK_TAG, "DS18B20 sens: %x.%x.%x.%x.%x.%x (CRC %x) -- %f (C)",
                     ow_dev.rom[i].code[0], ow_dev.rom[i].code[1], ow_dev.rom[i].code[2],
                     ow_dev.rom[i].code[3], ow_dev.rom[i].code[4], ow_dev.rom[i].code[5],
                     ow_dev.rom[i].crc, _temp);
          }
        }
      }
    } else {
      ESP_LOGI(OW_TASK_TAG, "There are no any device on 1-wire bus");
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  init();
  xTaskCreate(test_task, "test_wo_rmt_task", 2048, NULL, 10, NULL);
}
