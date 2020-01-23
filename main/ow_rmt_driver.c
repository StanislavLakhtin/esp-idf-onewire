//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#include "ow_rmt_driver.h"
#include "driver/rmt.h"

esp_err_t ow_rmt_driver_init() {
  rmt_config_t _config_tx = RMT_DEFAULT_CONFIG_TX(OW_RMT_TX_PIN, OW_RMT_TX_CHANNEL);
  _config_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  _config_tx.clk_div = CLK_DIV;
  ESP_ERROR_CHECK(rmt_config(&_config_tx) );
  ESP_ERROR_CHECK(
      rmt_driver_install(_config_tx.channel, 0,
                         ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED));

  rmt_config_t _config_rx = RMT_DEFAULT_CONFIG_RX(OW_RMT_RX_PIN, OW_RMT_RX_CHANNEL);
  _config_rx.rx_config.filter_en = true;
  _config_rx.rx_config.filter_ticks_thresh = OW_MS_TO_RMT_TICKS(_THRESHOLD_DURATION);
  _config_rx.clk_div = CLK_DIV;
  ESP_ERROR_CHECK(rmt_config(&_config_rx) );
  ESP_ERROR_CHECK(
      rmt_driver_install(_config_rx.channel,
                         sizeof(rmt_item32_t) * OW_RMT_RX_BUFFER_SIZE,
                         ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED));
  return ESP_OK;
}

static RingbufHandle_t get_rx_buffer() {
  RingbufHandle_t _rb = NULL;
  ESP_ERROR_CHECK(rmt_get_ringbuf_handle(OW_RMT_RX_CHANNEL, &_rb));
  return _rb;
}

esp_err_t _ow_rmt_write(uint32_t pulse_duration_ms) {
  // обычное состояние шины -- подтянуто к питанию. Создаём импульс, подтягивая к земле, нужной длинны
  rmt_item32_t _ticks = OW_TICK_SLOT_MS_LOW(pulse_duration_ms);
  return rmt_write_items(OW_RMT_TX_CHANNEL, &_ticks, 1, true);
}

uint32_t _ow_rmt_read() {
  uint16_t _presence = 0x00;
  ESP_ERROR_CHECK(rmt_rx_start(OW_RMT_RX_CHANNEL, false));
  RingbufHandle_t _rb = get_rx_buffer();
  uint32_t length;
  rmt_item32_t *_items = (rmt_item32_t *) xRingbufferReceive(_rb, &length, 10);
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
    // после разбора данных, сбрасываем ringbuffer.
    vRingbufferReturnItem(_rb, (void *) _items);
  }
  ESP_ERROR_CHECK(rmt_rx_stop(OW_RMT_RX_CHANNEL));
  return _presence;
}

// Метод формирует импульс заданной длительности и возвращает (читает) на шине 1-wire ответный импульс, длительность
// которого возвращает в качестве результата
uint32_t _ow_rmt_write_then_read(uint32_t pulse_duration_ms) {
  ESP_ERROR_CHECK(_ow_rmt_write(pulse_duration_ms));
  return _ow_rmt_read();
}

uint16_t ow_rmt_reset(void) {
  uint16_t old_thresh = 0x00;
  rmt_get_rx_idle_thresh(OW_RMT_RX_CHANNEL, &old_thresh);
  rmt_set_rx_idle_thresh(OW_RMT_RX_CHANNEL, OW_MS_TO_RMT_TICKS(_RESET_DURATION + _THRESHOLD_DURATION));
  uint16_t _presence = OW_TICKS_TO_MS(_ow_rmt_write_then_read(_RESET_DURATION));
  rmt_set_rx_idle_thresh(OW_RMT_RX_CHANNEL, old_thresh);
  ESP_LOGD("OW", "presence %d ms", _presence);
  if (_presence > _PRESENCE_LOWER_BORDER && _presence < _PRESENCE_HIGH_BORDER)
    return _presence;
  else
    return 0x00;
}

void ow_rmt_send_signal(uint16_t data) {
  uint32_t _duration_ms = data ? _WRITE_1_DURATION : _WRITE_0_DURATION;
  _ow_rmt_write(_duration_ms);
}

uint16_t ow_rmt_read_signal( void ) {
  return _ow_rmt_read();
}