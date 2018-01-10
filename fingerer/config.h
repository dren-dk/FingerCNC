#pragma once

#include <inttypes.h>

/*
| Index | Parameter           |     type | unit  | quantum   | Editor |
|-------|---------------------|----------|-------|-----------|--------|
|     0 | Gearing    	      | uint32_t |       | steps/mm  |    int |
|     1 | Blade width  	      | uint32_t |   mm  | step size |  fixed |
|     2 | Space/Finger size   | uint32_t |   mm  | step size |  fixed |
|     3 | Offset of home      | uint32_t |   mm  | step size |  fixed |
|     4 | A/B board           | uint8_t  |       | 1         |   enum |
|     5 | Space               | uint8_t  |       | 1         |    int |
|     6 | Stride	      | uint8_t  |    %  | 1         |    int |
|     7 | Min speed	      | uint32_t | mm/s  | step size |  fixed |
|     8 | Speed		      | uint32_t | mm/s  | step size |  fixed |
|     9 | Accel		      | uint8_t	 | mm/s² | step/ms²  |  fixed |
*/

typedef struct ConfigParam {
  uint32_t value;   // Raw value as used by the code
  const char *name; // Name of parameter in pgmspace
  const char *unit; // Base unit of the paramter in pgmspace
  uint32_t divisor; // Divisor used to turn the raw value into the base unit
  uint8_t decimals; // Decimals to show when editing this parameter
  uint32_t def;     // Default value 
  uint32_t min;     // Minimum value
  uint32_t max;     // Maximum value
} ConfigParam;

/*
 divisor = cp->divisor == 0 ? stepsPerMm : cp->divisor; 
 decimalDivisor = powi(10, cp->decimals);
 displayValueWithoutDecimalPoint = cp->value*decimalDivisor / divisor
 def, min and max are at the same scale as displayValueWithoutDecimalPoint
 
 sprintf(buffer, "%d", displayValueWithoutDecimalPoint);
 dp = strlen(buffer)-decimals;
*/

typedef enum {
  C_STEPS_PER_MM = 0,
  C_BLADE_WIDTH = 1,
  C_FINGER_WIDTH = 2,
  C_HOME_OFFSET = 3,
  C_BOARD = 4,
  C_SPACE = 5,
  C_STRIDE = 6,
  C_MIN_SPEED = 7,
  C_SPEED = 8,
  C_ACCELERATION = 9,

  // Insert config index enums above and don't reorder them EVER.
  CONFIGS_USED
} ConfigId;

void initConfig();
uint32_t getConfigValue(ConfigId configId);
ConfigParam* getConfigParam(ConfigId configId);

uint32_t displayToValue(ConfigParam* cp, uint32_t dv);
uint32_t valueToDisplay(ConfigParam* cp, uint32_t v);
