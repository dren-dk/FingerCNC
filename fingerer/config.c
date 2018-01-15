#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "uart.h"
#include "config.h"


const uint32_t EEPROM_MAGIC = 0x05AAFF00;

ConfigParam config[CONFIGS_USED];

uint32_t getConfigValue(ConfigId configId) {
  return getConfigParam(configId)->value;
}

ConfigParam* getConfigParam(ConfigId configId) {
  return config + configId;
}

uint32_t pow10(uint8_t power) {
  uint32_t r = 1;
  for (uint8_t i=0;i<power;i++) {
    r *= 10;
  }
  return r;
}

uint32_t displayToValue(ConfigParam* cp, uint32_t dv) {
  if (cp->divisor) {
    if (cp->divisor > 1) {
      dv *= cp->divisor;
    }
  } else {
    dv *= getConfigValue(C_STEPS_PER_MM);
  }

  if (cp->decimals) {
    dv /= pow10(cp->decimals);
  }

  return dv;
  
  // return (dv*cp->divisor)/pow10(cp->decimals);
}

uint32_t valueToDisplay(ConfigParam* cp, uint32_t v) {
  if (cp->decimals) {
    v *= pow10(cp->decimals);
  }
  
  if (cp->divisor) {
    if (cp->divisor > 1) {
      v /= cp->divisor;
    }
  } else {
    v /= getConfigValue(C_STEPS_PER_MM);
  }

  return v;

  // The above is a quite possibly slightly optimized version of this:
  // return v*pow10(cp->decimals) / divisor;
}

void ic(ConfigId id, const char *name, const char *unit, uint32_t divisor, uint8_t decimals,
	uint32_t def, uint32_t min, uint32_t max) {
  ConfigParam* cp = getConfigParam(id);
  cp->id = id;
  cp->name = name;
  cp->unit = unit;
  cp->divisor = divisor;
  cp->decimals = decimals;
  cp->def = def;
  cp->min = min;
  cp->max = max;

  uint32_t *eepromAddress = (uint32_t *)(4+id*4);
  cp->value = eeprom_read_dword(eepromAddress);
}

void storeConfig(ConfigParam *cp) {
  uint32_t *eepromAddress = (uint32_t *)(4+cp->id*4);
  eeprom_write_dword(eepromAddress, cp->value);
}  

const uint32_t STEPS_MM = 0;

const char MM[] PROGMEM = "mm";
const char MMPS[] PROGMEM = "mm/s";

void initConfig() {

  // ID              Name                  Unit              divisor   d  def    min   max
  ic(C_STEPS_PER_MM, PSTR("Gearing"),      PSTR("steps/mm"), 1,        0, 200,   25,   8000);
  ic(C_BLADE_WIDTH,  PSTR("Kerf"),         MM,               STEPS_MM, 3, 2000,  1000, 6000);
  ic(C_FINGER_WIDTH, PSTR("Finger Width"), MM,               STEPS_MM, 3, 1000,  1000, 50000);
  ic(C_HOME_OFFSET,  PSTR("Home Offset"),  MM,               STEPS_MM, 3, 0,     0,    60000); 
  ic(C_BOARD,        PSTR("Board"),        PSTR("A/B"),      1,        0, 0,     0,    1); 
  ic(C_SPACE,        PSTR("Space"),        PSTR("n"),        1,        0, 0,     0,    100);
  ic(C_STRIDE,       PSTR("Stride"),       PSTR("%"),        1,        0, 50,    5,    95);
  ic(C_MIN_SPEED,    PSTR("Min Speed"),    MMPS,             STEPS_MM, 2, 1250,  100,  50000);
  ic(C_SPEED,        PSTR("Speed"),        MMPS,             STEPS_MM, 2, 1250,  100,  50000);
  ic(C_ACCELERATION, PSTR("Acceleration"), PSTR("mm/s²"),    STEPS_MM, 0, 5,     50,   10000);
  
  if (eeprom_read_dword(0) != EEPROM_MAGIC) {
    L("Loading defaults\n");
    for (ConfigId i=0 ; i<CONFIGS_USED ; i++) {
      ConfigParam* cp = getConfigParam(i);
      cp->value = displayToValue(cp, cp->def);
      storeConfig(cp);
    }
    eeprom_write_dword(0, EEPROM_MAGIC);
  }
  
  for (ConfigId i=0 ; i<CONFIGS_USED ; i++) {
    ConfigParam* cp = getConfigParam(i);
    P("%S\t%ld\t%S\n", cp->name, valueToDisplay(cp, cp->value), cp->unit);
  }
}
