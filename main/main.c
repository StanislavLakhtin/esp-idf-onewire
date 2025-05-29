/* UART 1-wire (OneWire) Free RTOS Example for DS18B20
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
#include "ow/ow.h"
#include "ow/ds18b20.h"
#include "ow_uart_driver.h"

OneWire ow_dev = {
    .reset = ow_uart_reset,
    .write = ow_uart_send_signal,
    .read = ow_uart_read_signal
};
const char *OW_TASK_TAG = "1wire";

static void init() {
  ESP_ERROR_CHECK(ow_uart_driver_init());
}

static const char * SYMBOLS = "0123456789ABCDEF";

static void reverse_as_chars(uint8_t * from, char* to, int from_length) {
  uint8_t _p = 0;
  for (int i = from_length - 1 ; i >= 0 ; i--) {
    to[_p] = SYMBOLS [from[i] >> 4];
    to[_p+1] = SYMBOLS [0x0f & from[i]];
    _p += 2;
  }
  to[_p] = 0x00;
}

static void test_driver_task(void *arg) {
  char _mac[13];
  while (1) {
    uint16_t _presence = ow_dev.reset();
    if ( _presence ) {
      ESP_LOGI(OW_TASK_TAG, "PRESENCE detected on 1-wire bus. Presence signal: %d", _presence);
      ESP_LOGI(OW_TASK_TAG, "Scan OW bus...");
      if (ow_scan(&ow_dev)) {
        for (uint8_t i = 0; i < ow_dev.state.devicesQuantity; i++) {
          if (ow_dev.rom[i].family == 0x28) {  // Found DS18B20 Temp sensor
            float _temp = read_temperature(&ow_dev, &ow_dev.rom[i]);
            reverse_as_chars(ow_dev.rom[i].code, _mac, 6);
            ESP_LOGI(OW_TASK_TAG, "DS18B20[0x28] sens id: 0x%s (CRC %02X) -- %f (C)",
                     _mac, ow_dev.rom[i].crc, _temp);
          }
        }
      }
      ESP_LOGI(OW_TASK_TAG, "Presence correct. SCAN found %d devices on the bus", ow_dev.state.devicesQuantity);
    } else {
      ESP_LOGW(OW_TASK_TAG, "There are no any device on 1-wire bus. ");
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  init();
  xTaskCreate(test_driver_task, "test_driver_task", 8048, NULL, 10, NULL);
}
