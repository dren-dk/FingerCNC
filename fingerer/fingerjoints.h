#pragma once

#include <inttypes.h>

/*
  Pure math for making fingerjoints, no machine or framework specific code involved, to allow testing outside the project
*/

typedef struct FingerJoints {
  uint32_t fingerWidth;
  uint8_t board;
  uint32_t homePos;
  uint32_t kerf;
  uint32_t stridePct;

  uint8_t space;
    
  uint8_t cutsPerSlot;
  uint32_t advancePerCut;

  uint32_t spaceStartPos;
  
  uint8_t currentCut;
  
  uint32_t currentPos;
} FingerJoints;

/*
fingerWidth = getConfigParam(C_FINGER_WIDTH);
initFingerJoints(fingerWidth->value, getConfigValue(C_BOARD), getConfigValue(C_SPACE), getConfigValue(C_HOME_OFFSET),
                 getConfigValue(C_BLADE_WIDTH), getConfigValue(C_STRIDE));
*/

void initFingerJoints(FingerJoints *fj,
                      uint32_t fingerWidth, uint8_t board, uint8_t space, 
                      uint32_t homePos, uint32_t kerf, uint32_t stridePct);
void startSpace(FingerJoints *fj, uint8_t space);
void fingerAvance(FingerJoints *fj);
void flipBoard(FingerJoints *fj);

