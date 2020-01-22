//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#include "ow_rmt_driver.h"
#include "driver/rmt.h"

void ow_rmt_driver_init() {
  rmt_config_t _config_tx = RMT_DEFAULT_CONFIG_TX(OW_RMT_TX_PIN, OW_RMT_TX_CHANNEL);
  _config_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  _config_tx.clk_div = 2;
  ESP_ERROR_CHECK(rmt_config(&_config_tx));
  ESP_ERROR_CHECK(rmt_driver_install(_config_tx.channel, 0, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
  rmt_config_t _config_rx = RMT_DEFAULT_CONFIG_RX(OW_RMT_RX_PIN, OW_RMT_RX_CHANNEL);
  _config_rx.rx_config.filter_en = false;
  _config_rx.clk_div = _config_tx.clk_div;
  ESP_ERROR_CHECK(rmt_config(&_config_rx));
  ESP_ERROR_CHECK(rmt_driver_install(_config_rx.channel, _config_rx.mem_block_num, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
}

rmt_tx_end_callback_t _pre_fn_callback;

static void reset_callback(rmt_channel_t channel, void *arg) {
  RingbufHandle_t rb = NULL;
  rmt_item32_t *items = NULL;
  uint32_t length = 0;
  // get RMT RX ringbuffer
  rmt_get_ringbuf_handle(OW_RMT_RX_CHANNEL, &rb);
  rmt_rx_start(OW_RMT_RX_CHANNEL, true);
  while (rb) {
    items = (rmt_item32_t *) xRingbufferReceive(rb, &length, 1000);
    if (items) {
      //after parsing the data, return spaces to ringbuffer.
      vRingbufferReturnItem(rb, (void *) items);
    } else {
      break;
    }
  }
  rmt_rx_stop(OW_RMT_RX_CHANNEL);
}

uint16_t ow_rmt_reset(void) {
  _pre_fn_callback = rmt_register_tx_end_callback(reset_callback, NULL);
  ESP_ERROR_CHECK(rmt_write_items(OW_RMT_TX_CHANNEL, &RESET_PULSE, 1, true));
  return 0x00;
}