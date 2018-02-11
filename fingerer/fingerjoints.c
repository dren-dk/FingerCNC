#include "fingerjoints.h"

void initFingerJoints(FingerJoints *fj,
                      uint32_t fingerWidth, uint8_t board, uint8_t space, 
                      uint32_t homePos, uint32_t kerf, uint32_t stridePct) {
  fj->fingerWidth = fingerWidth;
  fj->board = board;
  fj->space = space;
  fj->homePos = homePos;
  fj->kerf = kerf;
  fj->stridePct = stridePct;
  
  fj->spaceStartPos = 0;
  fj->cutsPerSlot = 0;
  fj->advancePerCut = 0;
  
  // First advance is just the ideal one based on the config
  fj->advancePerCut = (fj->kerf * fj->stridePct) / 100;
  uint32_t widthPastFirstKerf = fj->fingerWidth - fj->kerf;
  fj->cutsPerSlot = widthPastFirstKerf / fj->advancePerCut;
  if (fj->cutsPerSlot*fj->advancePerCut < widthPastFirstKerf) {
    fj->cutsPerSlot++;
  }

  // This is the actual distance to move for each cut after the first one
  fj->advancePerCut = widthPastFirstKerf / fj->cutsPerSlot;      
  
  startSpace(fj, fj->space);
}


// Move to a specific space
void startSpace(FingerJoints *fj, uint8_t space) {
  fj->space = space;
  fj->currentCut = 0;
  fj->currentPos = fj->spaceStartPos = fj->homePos+ (fj->board+fj->space*2)*fj->fingerWidth;  
}

void flipBoard(FingerJoints *fj) {
  fj->board = fj->board ? 0 : 1;
  startSpace(fj, fj->space);
}

// Advance to the next cut
void fingerAvance(FingerJoints *fj) {
  if (++fj->currentCut >= fj->cutsPerSlot) {
    startSpace(fj, ++fj->space);
  } else {
    fj->currentPos = fj->spaceStartPos + fj->currentCut*fj->advancePerCut;
  }
}
