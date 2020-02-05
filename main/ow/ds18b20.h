/**
    @author Stanislav Lakhtin
    @date   14.11.2019
    @brief  This example code is in the Public Domain (or CC0 licensed, at your option.)

            Unless required by applicable law or agreed to in writing, this
            software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
            CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef BBB_DS18B20_H
#define BBB_DS18B20_H

#include "ow.h"

#define RESOLUTION_9_BIT  0x1f
#define RESOLUTION_10_BIT 0x3f
#define RESOLUTION_11_BIT 0x5f
#define RESOLUTION_12_BIT 0x7f

#define DS18B20_LSB     0x00
#define DS18B20_MSB     0x01
#define DS18B20_TH      0x02
#define DS18B20_TL      0x03
#define DS18B20_CONFIG  0x04
#define DS18B20_BYTE_5  0x05
#define DS18B20_BYTE_6  0x06
#define DS18B20_BYTE_7  0x07
#define DS18B20_CRC     0x08

#define DS18B20_CONVERT_TEMPERATURE 0x44
#define DS18B20_READ_SCRATCHPAD 0xBE
#define DS18B20_WRITE_SCRATCHPAD 0x4E
#define DS18B20_COPY_SCRATCHPAD 0x48
#define DS18B20_RECALL_E2 0xB8
#define DS18B20_READ_POWER_SUPPLY 0xB4

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Note that if the DS18B20 alarm function is not used, the TH and TL registers can serve as general-purpose memory.
 * Data is written to bytes 2, 3, and 4 of the scratchpad using the Write Scratchpad [4Eh] command;
 * the data must be transmitted to the DS18B20 starting with the least signifi- cant bit of byte 2.
 * To verify data integrity, the scratchpad can be read (using the Read Scratchpad [BEh] command) after the data is written.
 *
 * Data in the EEPROM registers is retained when the device is powered down;
 * at power-up the EEPROM data is reloaded into the corresponding scratchpad locations.
 * Data can also be reloaded from EEPROM to the scratch- pad at any time using the Recall E2 [B8h] command.
 * The master can issue read time slots following the Recall E2 command and the DS18B20 will indicate the status
 * of the recall by transmitting 0 while the recall is in progress and 1 when the recall is done.
 * */

void write_scratchpad(OneWire *ow_dev, RomCode *rom, int8_t tH, int8_t tL, uint8_t conf);

void read_scratchpad(OneWire *ow_dev, RomCode *rom, uint8_t *scratchpad, uint8_t len);

void store_current_scratchpad_to_eeprom(OneWire *ow_dev, RomCode *rom);

void restore_scratchpad_from_eeprom(OneWire *ow_dev, RomCode *rom);

float read_temperature(OneWire *ow_dev, RomCode *rom);

uint8_t read_power_supply(OneWire *ow_dev, RomCode *rom);

#ifdef __cplusplus
}
#endif

#endif //BBB_DS18B20_H
