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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ow_uart_driver.h"

OneWire ow_dev;

void ow_task(void *arg) {
  ow_dev.usart_setup = ow_uart_driver_setup;
  ow_dev.send_usart = ow_uart_driver_send;
  while (1) {
    printf("Scan OW bus...\n");
    uint8_t rslt = ow_scan(&ow_dev);
    if (rslt) {
      printf("Found one or more devices\n");
      for (uint8_t i = 0; i < ow_dev.state.devicesQuantity; i++) {
        if (ow_dev.rom[i].family == 0x28) {  // Found DS18B20 Temp sensor
          printf("temp: %f", read_temperature(&ow_dev, &ow_dev.rom[1]));
        }
      }
    } else {
      printf("There are no any OW device on the bus\n");
    }
    for (int i = 10; i >= 0; i--) {
      printf("Rescaning in %d seconds...\n", i * 1);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    fflush(stdout);
  }
}

void app_main(void) {
  xTaskCreate(ow_task, "ow_task", 2048, NULL, 10, NULL);
}
