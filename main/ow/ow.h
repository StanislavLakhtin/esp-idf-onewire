/**
    @author Stanislav Lakhtin
    @date   11.07.2016
    @brief  This example code is in the Public Domain (or CC0 licensed, at your option.)

            Unless required by applicable law or agreed to in writing, this
            software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
            CONDITIONS OF ANY KIND, either express or implied.
*/
#ifndef STM32_DS18X20_ONEWIRE_H
#define STM32_DS18X20_ONEWIRE_H

#include <stdint.h>

#define ONEWIRE_SEARCH 0xF0
#define ONEWIRE_SKIP_ROM 0xCC
#define ONEWIRE_READ_ROM 0x33
#define ONEWIRE_MATCH_ROM 0x55

#ifndef ONEWIRE_MAXDEVICES_ON_THE_BUS
#define ONEWIRE_MAXDEVICES_ON_THE_BUS 10
#endif

#define DS18B20 0x28
#define DS18S20 0x10

#define WIRE_0    0x00 // 0x00 --default
#define WIRE_1    0xff


#ifndef FALSE
#define FALSE 0x00
#endif
#ifndef TRUE
#define TRUE  0x01
#endif

#define ALERT 0x02

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
  uint8_t family;
  uint8_t code[6];
  uint8_t crc;
} RomCode;

typedef void ( * ow_send_fptr_t ) ( uint16_t data );
typedef uint16_t ( * ow_reset_fptr_t ) ( void );
typedef uint16_t ( * ow_read_fptr_t ) ( void );

typedef struct {
  uint8_t ROM_BUFFER[8];
  uint8_t lastDiscrepancy;
  uint8_t lastFamilyDiscrepancy;
  uint8_t lastDeviceFlag;
  uint8_t hasMoreROM; // flag if MAXDEVICES_ON_THE_BUS is not enough
  uint8_t devicesQuantity;
  uint8_t crc8;
} OWState;

typedef struct {
  RomCode rom[ONEWIRE_MAXDEVICES_ON_THE_BUS];
  ow_send_fptr_t write;
  ow_reset_fptr_t reset;
  ow_read_fptr_t read;
  OWState state;
} OneWire;

void ow_clear_state(OneWire *ow_dev);
uint8_t ow_find_next_ROM(OneWire *ow_dev, uint8_t search_command);
uint8_t ow_scan(OneWire *ow_dev);
uint8_t ow_read_bit(OneWire *ow_dev);
void ow_send_byte(OneWire *ow_dev, uint8_t data);
void ow_match_rom(OneWire *ow_dev, RomCode *rom);

#ifdef __cplusplus
}
#endif

#endif //STM32_DS18X20_ONEWIRE_H