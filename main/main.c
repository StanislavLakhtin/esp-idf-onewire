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
#include "ow_uart_driver.h"

OneWire ow_dev;
const char *OW_TASK_TAG = "1wire";

static void uart_event_task(void *pvParameters) {
  uart_event_t event;
  uint8_t *dtmp = (uint8_t *) malloc(BUF_SIZE);
  for (;;) {
    //Waiting for UART event.
    if (xQueueReceive(ow_uart_queue, (void *) &event, (portTickType) portMAX_DELAY)) {
      memset(dtmp, 0x00, BUF_SIZE);
      ESP_LOGI(OW_TASK_TAG, "uart[%d] event:", OW_UART);
      switch (event.type) {
        case UART_DATA:
          ESP_LOGI(OW_TASK_TAG, "[UART DATA]: %d", event.size);
          uart_read_bytes(OW_UART, dtmp, event.size, portMAX_DELAY);
          for (int i = 0; i < event.size; i++) {
            ow_bus_get_echo_data(&ow_dev, dtmp[i]);
          }
          break;
          //Event of HW FIFO overflow detected
        case UART_FIFO_OVF:
          ESP_LOGI(OW_TASK_TAG, "hw fifo overflow");
          // If fifo overflow happened, you should consider adding flow control for your application.
          // The ISR has already reset the rx FIFO,
          // As an example, we directly flush the rx buffer here in order to read more data.
          uart_flush_input(OW_UART);
          xQueueReset(ow_uart_queue);
          break;
          //Event of UART ring buffer full
        case UART_BUFFER_FULL:
          ESP_LOGI(OW_TASK_TAG, "ring buffer full");
          // If buffer full happened, you should consider encreasing your buffer size
          // As an example, we directly flush the rx buffer here in order to read more data.
          uart_flush_input(OW_UART);
          xQueueReset(ow_uart_queue);
          break;
        default:
          ESP_LOGI(OW_TASK_TAG, "uart event type: %d", event.type);
          break;
      }
    }
  }
  free(dtmp);
  dtmp = NULL;
  vTaskDelete(NULL);
}

static void init() {
  ow_uart_driver_init();
  ow_dev.usart_setup = ow_uart_baudrate_setup;
  ow_dev.send_usart = ow_uart_driver_send;
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
  xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
  xTaskCreate(ow_task, "ow_task", 2048, NULL, 10, NULL);
}
