//
// Created by Stanislav Lakhtin on 21/01/2020.
//

#include "ow_rmt_driver.h"
#include "driver/rmt.h"

void ow_rmt_driver_init() {
  rmt_config_t _config_tx = RMT_DEFAULT_CONFIG_TX( OW_RMT_TX_PIN, OW_RMT_TX_CHANNEL );
  _config_tx.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  _config_tx.clk_div = CLK_DIV;
  ESP_ERROR_CHECK( rmt_config( &_config_tx ));
  ESP_ERROR_CHECK(
      rmt_driver_install( _config_tx.channel, 0,
                          ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED ));
  rmt_config_t _config_rx = RMT_DEFAULT_CONFIG_RX( OW_RMT_RX_PIN, OW_RMT_RX_CHANNEL );
  _config_rx.rx_config.filter_en = false;
  _config_rx.clk_div = CLK_DIV;
  _config_rx.rx_config.idle_threshold = _IDLE_DURATION;
  ESP_ERROR_CHECK( rmt_config( &_config_rx ));
  ESP_ERROR_CHECK(
      rmt_driver_install( _config_rx.channel, ( sizeof(rmt_item32_t) + 8 ) * OW_RMT_RX_BUFFER_SIZE, // из-за RINGBUF_TYPE_NOSPLIT в RingbufferType_t
                          ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED ));
}

static RingbufHandle_t get_rx_buffer() {
  RingbufHandle_t _rb = NULL;
  ESP_ERROR_CHECK( rmt_get_ringbuf_handle( OW_RMT_RX_CHANNEL, &_rb ));
  return _rb;
}

// flush any pending/spurious traces from the RX channel
static void onewire_flush_rmt_rx_buf() {
  void *p;
  size_t s;
  RingbufHandle_t * _rb = get_rx_buffer();

  while ((p = xRingbufferReceive(_rb, &s, 0))) {
    vRingbufferReturnItem(_rb, p);
  }
}

uint16_t ow_rmt_reset( void ) {
  uint16_t _presence = 0x00;
  // устанавливаем threshold на несребатывание чтения для того, чтобы
  // пропустить 1-wire цикл RESET (480) и паузу после него (70ms)
  ESP_ERROR_CHECK(
      rmt_set_rx_idle_thresh( OW_RMT_RX_CHANNEL, OW_MS_TO_RMT_TICKS( _RESET_DURATION + _RESET_IDLE_DURATION )));
  // инициируем цикл чтения с очисткой кольцевого буфера чтения
  // todo check onewire_flush_rmt_rx_buf();
  //ESP_ERROR_CHECK( rmt_rx_start( OW_RMT_RX_CHANNEL, true ));
  // обычное состояние шины -- подтянуто к питанию. Создаём импульс, подтягивая к земле, нужной длинны
  rmt_item32_t reset_ticks = OW_TICK_SLOT_MS_LOW( _RESET_DURATION );
  ESP_LOGI("OW", "RESET duration: %d ticks ( %d ms, CLK_DIV = %d)", reset_ticks.duration0, _RESET_DURATION, CLK_DIV);
  if ( rmt_write_items( OW_RMT_TX_CHANNEL, &reset_ticks, 1, true ) == ESP_OK ) {
    /*RingbufHandle_t _rb = get_rx_buffer();
    uint32_t length;
    rmt_item32_t * _items = ( rmt_item32_t * ) xRingbufferReceive( _rb, &length, 10 / portTICK_PERIOD_MS);
    if ( _items ) {
      // мы что-то получили. Теперь следует проверить, что прилетело
      // каждый слот представляет собой около 8ms.
      // Если хотя бы в одном есть 0, означает на шине кто-то есть кроме мастера
      length /= 4; // one RMT = 4 Bytes
      for ( int i = 0 ; i < length ; i++ ) {
        if ((( _items[ i ].level0 == 0 ) && ( _items[ i ].duration0 > 0 )) ||
            (( _items[ i ].level1 == 0 ) && ( _items[ i ].duration1 > 0 ))) {
          // если был хотя бы один слот с 0
          _presence += 0x01;
        }
      }
      // после разбора данных, сбрасываем ringbuffer.
      vRingbufferReturnItem( _rb, ( void * ) _items );
    }*/
  }
  //ESP_ERROR_CHECK( rmt_rx_stop( OW_RMT_RX_CHANNEL ));
  return _presence;  // вернём ненулевое значение в виде длительности PRESENCE в тиках
}