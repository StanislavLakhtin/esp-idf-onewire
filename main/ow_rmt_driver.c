//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#include "ow_rmt_driver.h"
#include "driver/rmt.h"

void ow_rmt_driver_init() {
  rmt_config_t _config_tx = RMT_DEFAULT_CONFIG_TX(OW_RMT_TX_PIN, OW_RMT_TX_CHANNEL);
  _config_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  ESP_ERROR_CHECK(rmt_config(&_config_tx));
  ESP_ERROR_CHECK(rmt_driver_install(_config_tx.channel, 0, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
  rmt_config_t _config_rx = RMT_DEFAULT_CONFIG_RX(OW_RMT_RX_PIN, OW_RMT_RX_CHANNEL);
  _config_rx.rx_config.filter_en = false;
  _config_rx.clk_div = 10;
  _config_rx.rx_config.idle_threshold = _IDLE_DURATION;
  ESP_ERROR_CHECK(rmt_config(&_config_rx));
  ESP_ERROR_CHECK(rmt_driver_install(_config_rx.channel, OW_RMT_RX_BUFFER_SIZE, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
}

uint16_t ow_rmt_reset(void) {
  uint16_t _rx_tr;
  rmt_get_rx_idle_thresh(OW_RMT_RX_CHANNEL, &_rx_tr);
  rmt_set_rx_idle_thresh(OW_RMT_RX_CHANNEL, _RESET_DURATION + _RESET_IDLE_DURATION);
  rmt_rx_start(OW_RMT_RX_CHANNEL, true);
  if (rmt_write_items(OW_RMT_TX_CHANNEL, &RESET_PULSE, 1, true)) {
    size_t _rx_size = 0x00;
    rmt_item32_t* rx_items = (rmt_item32_t *)xRingbufferReceive(, &rx_size, 100 / portTICK_PERIOD_MS);
  }
  rmt_rx_stop(OW_RMT_RX_CHANNEL);
  return 0x00;
}