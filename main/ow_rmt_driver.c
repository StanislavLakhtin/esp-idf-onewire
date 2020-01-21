//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#include "ow_rmt_driver.h"
#include "driver/rmt.h"

void ow_rmt_driver_init() {
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(OW_RMT_PIN, OW_RMT_CHANNEL);
  config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  config.clk_div = 2;
  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
}

uint16_t ow_rmt_reset(void) {
  ESP_ERROR_CHECK(rmt_write_items(OW_RMT_CHANNEL, &RESET_PULSE, 1, true));
  return 0x00;
}