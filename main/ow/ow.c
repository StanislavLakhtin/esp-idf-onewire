#include "ow.h"

/**
    @author Stanislav Lakhtin
    @date   12.07.2019
    @brief  This example code is in the Public Domain (or CC0 licensed, at your option.)

            Unless required by applicable law or agreed to in writing, this
            software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
            CONDITIONS OF ANY KIND, either express or implied.
*/

static uint8_t docrc8(OneWire *ow_dev, uint8_t value);

void ow_clear_state(OneWire *ow_dev) {
  OWState *state = &ow_dev->state;
  state->hasMoreROM = FALSE;
  state->lastDiscrepancy = 0x00;
  state->lastFamilyDiscrepancy = 0x00;
  state->lastDeviceFlag = FALSE;
  state->devicesQuantity = 0x00;
  state->crc8 = 0x00;
  for (uint8_t i = 0; i < ONEWIRE_MAXDEVICES_ON_THE_BUS; i++) {
    RomCode *ROM = &ow_dev->rom[i];
    ROM->crc = 0x00;
    ROM->family = 0x00;
    for (uint8_t n = 0; n < 6; n++)
      ROM->code[n] = 0x00;
  }
}

//void ow_send(OneWire *ow_dev, uint8_t data) {
//  ow_dev->write(data);
//}

void ow_send_byte(OneWire *ow_dev, uint8_t data) {
  for (uint8_t b = 0; b < 8; b++) {
    uint8_t bit_mask = (data & 0x01) ? WIRE_1 : WIRE_0;
    ow_dev->write(bit_mask);
    data >>= 1;
  }
}

uint8_t ow_read_bit(OneWire *ow_dev) {
  return ow_dev->read() ? 0x01 : 0x00;
}

uint8_t ow_scan(OneWire *ow_dev) {
  uint8_t rslt, num = 0;
  ow_clear_state(ow_dev);
  do {
    rslt = ow_find_next_ROM(ow_dev, ONEWIRE_SEARCH);
    if (rslt == TRUE) {
      // нашёлся очередной ROM на шине
      if (num < ONEWIRE_MAXDEVICES_ON_THE_BUS) {
        ow_dev->rom[num].family = ow_dev->state.ROM_BUFFER[0];
        ow_dev->rom[num].crc = ow_dev->state.ROM_BUFFER[7];
        for (uint8_t n = 1; n < 7; n++)
          ow_dev->rom[num].code[n - 1] = ow_dev->state.ROM_BUFFER[n];
        ow_dev->state.devicesQuantity += 1;
      } else {
        // сохраняем флаг о том, что есть ROM на шине, который не влез в нашу таблицу и выходим с ошибкой
        ow_dev->state.hasMoreROM = TRUE;
        return ALERT;
      }
      num += 1;
    }
  } while (rslt && !ow_dev->state.lastDeviceFlag);
  return rslt;
}

/**
 * @param ow_dev
 * @param search_command ONEWIRE_SEARCH (default -- 0xF0) or ALARM_SEARCH (0xEC)
 * @return FALSE if no or TRUE if some device respond on the bus
 */
uint8_t ow_find_next_ROM(OneWire *ow_dev, uint8_t search_command) {
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;

  // initialize for search
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;

  OWState *state = &ow_dev->state;

  // если крайний вызов был не для крайнего устройства на шине
  if (!state->lastDeviceFlag) {
    // 1-Wire reset
    if ( !ow_dev->reset() ) {
      // сброс поиска
      state->lastDiscrepancy = 0;
      state->lastDeviceFlag = FALSE;
      state->lastFamilyDiscrepancy = 0;
      return FALSE;
    }

    ow_send_byte(ow_dev, search_command);

    do {
      // чтение прямого и комплиментарного битов
      id_bit      = ow_read_bit(ow_dev);
      cmp_id_bit  = ow_read_bit(ow_dev);

      // проверка, что на шине нет ни одного устрйоства. В этом случае и прямой и комплиментарный биты равны
      if (id_bit && cmp_id_bit )
        break;
      else {
        // все устройства имеют одинаковый ответ: 0 or 1, не важно сейчас
        if (id_bit != cmp_id_bit)
          search_direction = id_bit;  // выбираем в качестве "направления" прямой бит
        else {
          // В случае несоответствия (у некоторых устройств 0, а у других 1), проверяем, чтобы несоответствие было
          // ДО зафиксированного положения в Last Discrepancy
          // сдеанного на предыдущем шаге
          if (id_bit_number < state->lastDiscrepancy)
            search_direction = ((state->ROM_BUFFER[rom_byte_number] & rom_byte_mask) > 0);
          else
            // если равны в последнем сравнении, выбираем 1, или 0 в противном случае
            search_direction = (id_bit_number == state->lastDiscrepancy);

          // Если было 0, то записываем эту позицию в LastZero
          if (search_direction == 0) {
            last_zero = id_bit_number;

            // Проверяем, чтобы last_zero было последней в выборе семейства устройств
            if (last_zero < 9)
              state->lastFamilyDiscrepancy = last_zero;
          }
        }

        // устанавливаем (или стираем) бит в ROM
        // с масской rom_byte_mask
        if (search_direction)
          state->ROM_BUFFER[rom_byte_number] |= rom_byte_mask;
        else
          state->ROM_BUFFER[rom_byte_number] &= ~rom_byte_mask;

        // отсылаем на шину выбранное нами направление сканирования
        uint8_t answerBit = (uint8_t) ((search_direction == 0) ? WIRE_0 : WIRE_1);
        ow_dev->write(answerBit);

        // выполняем инкремент бита id_bit_number
        // и сдвигаем маску rom_byte_mask
        id_bit_number += 1;
        rom_byte_mask <<= 1;

        // Если маска установлена в 0 идём в новый байт SerialNum rom_byte_number и сбрасываем маску
        if (rom_byte_mask == 0) {
          docrc8(ow_dev, state->ROM_BUFFER[rom_byte_number]);  // походу рассчитываем CRC
          rom_byte_number += 1;
          rom_byte_mask = 0x01;
        }
      }
    } while (rom_byte_number < 8);  // сканировать будем все ROM байты (от 0-7)

    // Если поиск был успешным
    if (!((id_bit_number < 65) || (state->crc8 != 0))) {
      // считаем, что поиск очередного ROM прошёл успешно и устанавливаем LastDiscrepancy, LastDeviceFlag, search_result
      state->lastDiscrepancy = last_zero;

      // проверяем, является ли результат поиска последним на шине
      if (state->lastDiscrepancy == 0)
        state->lastDeviceFlag = TRUE;

      search_result = TRUE;
    }
  }

  // Если не было найдено ни одного устройства на шине :-( то мы считаем, что шина чистая и следующий поиск будет как в первый раз
  if (!search_result || !state->ROM_BUFFER[0]) {
    ow_clear_state(ow_dev);
    search_result = FALSE;
  }

  return search_result;
}

static const uint8_t onewire_crc_table[] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
};

static uint8_t docrc8(OneWire *ow_dev, uint8_t value) {
  ow_dev->state.crc8 = onewire_crc_table[ow_dev->state.crc8 ^ value];
  return ow_dev->state.crc8;
}

void ow_match_rom(OneWire *ow_dev, RomCode *rom) {
  if ( !ow_dev->reset() )
    return;
  ow_send_byte(ow_dev, ONEWIRE_MATCH_ROM);
  ow_send_byte(ow_dev, rom->family);
  for (uint8_t i = 0; i < 6; i++)
    ow_send_byte(ow_dev, rom->code[i]);
  ow_send_byte(ow_dev, rom->crc);
}

