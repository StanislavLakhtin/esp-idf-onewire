/**
    @author Stanislav Lakhtin
    @date   14.11.2019
    @brief  This example code is in the Public Domain (or CC0 licensed, at your option.)

            Unless required by applicable law or agreed to in writing, this
            software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
            CONDITIONS OF ANY KIND, either express or implied.
*/

#include "ds18b20.h"

/**
 * This command allows the master to write 3 bytes of data to the DS18B20’s scratchpad.
 * The first data byte is written into the TH register (byte 2 of the scratchpad),
 * the second byte is written into the TL register (byte 3), and the third byte
 * is written into the configuration register (byte 4).
 *
 * Data must be transmitted least significant bit first.
 * All three bytes MUST be written ( 0x48 command -- store_current_scratchpad_to_eeprom(...) ) before the master
 * issues a reset, or the data may be corrupted.
 * @param ow_dev
 * @param rom
 * @param tH  int8_t
 * @param tL  int8_t
 * @param conf
 */
void write_scratchpad(OneWire *ow_dev, RomCode *rom, int8_t tH, int8_t tL, uint8_t conf) {
  ow_match_rom(ow_dev, rom);
  ow_send_byte(ow_dev, DS18B20_WRITE_SCRATCHPAD);
  // This command allows the master to write 3 bytes of data to the DS18B20’s scratchpad.
  ow_send_byte(ow_dev, tH);
  ow_send_byte(ow_dev, tL);
  ow_send_byte(ow_dev, conf);
}

/**
 * This command allows the master to read the contents of the scratchpad.
 * The data transfer starts with the least sig- nificant bit of byte 0 and continues through
 * the scratchpad until the 9th byte (byte 8 – CRC) is read.
 *
 * The master may issue a reset to terminate reading at any time if only part of the scratchpad data is needed.
 * */
void read_scratchpad(OneWire *ow_dev, RomCode *rom, uint8_t *scratchpad, uint8_t len) {
  ow_match_rom(ow_dev, rom);
  ow_send_byte(ow_dev, DS18B20_READ_SCRATCHPAD);
  uint8_t l = len * 8, i = 0;
  while (i < l) {
    uint8_t p = (uint8_t) (i / 8);
    if (ow_read_bit(ow_dev))
      scratchpad[p] |= 1 << i % 8;
    else
      scratchpad[p] &= ~(1 << i % 8);
    i += 1;
  }
}

/**
 * This command copies the contents of the scratchpad TH, TL and configuration registers (bytes 2, 3 and 4) to EEPROM.
 */
void store_current_scratchpad_to_eeprom(OneWire *ow_dev, RomCode *rom) {
  ow_match_rom(ow_dev, rom);
  ow_send_byte(ow_dev, DS18B20_COPY_SCRATCHPAD);
}

/**
 * This command recalls the alarm trigger values (TH and TL) and configuration data from EEPROM and places
 * the data in bytes 2, 3, and 4, respectively, in the scratchpad memory.
 *
 * The recall operation happens automatically at power-up, so valid data is available in the scratchpad
 * as soon as power is applied to the device.
 * @param ow_dev
 * @param rom
 */
void restore_scratchpad_from_eeprom(OneWire *ow_dev, RomCode *rom) {
  ow_match_rom(ow_dev, rom);
  ow_send_byte(ow_dev, DS18B20_RECALL_E2);
}

/**
 * To avoid overcode this method return current temp and immediately send command to convert new
 * @param ow_dev
 * @param rom
 * @param temp buffer for result
 */
float read_temperature(OneWire *ow_dev, RomCode *rom) {
  uint8_t buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  read_scratchpad(ow_dev, rom, buffer, 5);  // We can cut off answer on necessary bytes
  ow_match_rom(ow_dev, rom);
  ow_send_byte(ow_dev,
               DS18B20_CONVERT_TEMPERATURE);  // conversion will be processed with stored before accuracy (9-12 bit)
  float rslt = (float) (((buffer[DS18B20_MSB] << 4) & 0xf0) | (buffer[DS18B20_LSB] >> 4));
  switch (buffer[DS18B20_CONFIG] | 0x0f) {
    default:
    case RESOLUTION_9_BIT:
      buffer[DS18B20_LSB] = buffer[DS18B20_LSB] & 0x08;
      break;
    case RESOLUTION_10_BIT:
      buffer[DS18B20_LSB] = buffer[DS18B20_LSB] & 0x0c;
      break;
    case RESOLUTION_11_BIT:
      buffer[DS18B20_LSB] = buffer[DS18B20_LSB] & 0x0e;
      break;
    case RESOLUTION_12_BIT:
      buffer[DS18B20_LSB] = buffer[DS18B20_LSB] & 0x0f;
      break;
  }
  rslt = rslt + (buffer[DS18B20_LSB]) / 16.0;
  return rslt;
}


/**
 * The master device issues this command followed by a read time slot to determine
 * if any DS18B20s on the bus are using parasite power.
 *
 * During the read time slot, parasite powered DS18B20s will pull the bus low,
 * and externally powered DS18B20s will let the bus remain high.
 *
 * @param ow_dev
 * @param rom
 * @return if ! 0x00 then device is parasite power
 */
uint8_t read_power_supply(OneWire *ow_dev, RomCode *rom) {
  ow_match_rom(ow_dev, rom);
  ow_send_byte(ow_dev, DS18B20_READ_POWER_SUPPLY);
  return ow_read_bit(ow_dev);
}