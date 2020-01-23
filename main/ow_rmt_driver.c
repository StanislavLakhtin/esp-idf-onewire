//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#include "ow_rmt_driver.h"
#include "driver/rmt.h"

void ow_rmt_driver_init() {
  rmt_config_t _config_tx = RMT_DEFAULT_CONFIG_TX(OW_RMT_TX_PIN, OW_RMT_TX_CHANNEL);
  _config_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  _config_tx.clk_div = CLK_DIV;
  ESP_ERROR_CHECK(rmt_config(&_config_tx));
  ESP_ERROR_CHECK(
      rmt_driver_install(_config_tx.channel, 0,
                         ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED));
  rmt_config_t _config_rx = RMT_DEFAULT_CONFIG_RX(OW_RMT_RX_PIN, OW_RMT_RX_CHANNEL);
  _config_rx.rx_config.filter_en = false;
  _config_rx.clk_div = CLK_DIV;
  _config_rx.rx_config.idle_threshold = _IDLE_DURATION;
  ESP_ERROR_CHECK(rmt_config(&_config_rx));
  ESP_ERROR_CHECK(
      rmt_driver_install(_config_rx.channel,
                         sizeof(rmt_item32_t) * OW_RMT_RX_BUFFER_SIZE, // из-за RINGBUF_TYPE_NOSPLIT в RingbufferType_t
                         ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED));
}

static RingbufHandle_t get_rx_buffer() {
  RingbufHandle_t _rb = NULL;
  ESP_ERROR_CHECK(rmt_get_ringbuf_handle(OW_RMT_RX_CHANNEL, &_rb));
  return _rb;
}

// Метод формирует импульс заданной длительности и возвращает (читает) на шине 1-wire ответный импульс, длительность
// которого возвращает в качестве результата
uint16_t ow_rmt_write_then_read(uint32_t pulse_duration_ms) {
  uint16_t _presence = 0x00;
  // обычное состояние шины -- подтянуто к питанию. Создаём импульс, подтягивая к земле, нужной длинны
  rmt_item32_t _ticks = OW_TICK_SLOT_MS_LOW(pulse_duration_ms);
  ESP_LOGD("OW", "generated pulse duration: %d ticks ( %d ms, CLK_DIV = %d)", _ticks.duration0, pulse_duration_ms,
           CLK_DIV);
  if (rmt_write_items(OW_RMT_TX_CHANNEL, &_ticks, 1, true) == ESP_OK) {
    ESP_ERROR_CHECK(rmt_rx_start(OW_RMT_RX_CHANNEL, true));
    RingbufHandle_t _rb = get_rx_buffer();
    uint32_t length;
    rmt_item32_t *_items = (rmt_item32_t *) xRingbufferReceive(_rb, &length, 10 / portTICK_PERIOD_MS);
    if (_items) {
      length /= 4; // one RMT = 4 Bytes
      ESP_LOGD("OW", "Bus response has got %d elements", length);
      for (int i = 0; i < length; i++) {
        ESP_LOGD("OW", "_tick[%d]: { %d / %d ; %d / %d }", i, _items[i].duration0, _items[i].level0,
                 _items[i].duration1, _items[i].level1);
        if ((_items[i].level0 == 0) && (_items[i].duration0 > _presence)) {
          ESP_LOGD("OW", "signal detected on items[%d].duration0 = %d ticks", i, _items[i].duration0);
          _presence = _items[i].duration0;
        }
        if ((_items[i].level1 == 0) && (_items[i].duration1 > _presence)) {
          ESP_LOGD("OW", "signal detected on items[%d].duration1 = %d ticks", i, _items[i].duration1);
          _presence = _items[i].duration1;
        }
      }
    }
    // после разбора данных, сбрасываем ringbuffer.
    vRingbufferReturnItem(_rb, (void *) _items);
  }
  ESP_ERROR_CHECK(rmt_rx_stop(OW_RMT_RX_CHANNEL));
  return _presence;
}

uint16_t ow_rmt_reset(void) {
  uint16_t _presence = OW_TICKS_TO_MS(ow_rmt_write_then_read(_RESET_DURATION));
  if (_presence > _PRESENCE_LOWER_BORDER && _presence < _PRESENCE_HIGH_BORDER)
    return _presence;
  else
    return 0x00;
}