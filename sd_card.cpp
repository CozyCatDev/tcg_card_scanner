#include "sd_card.h"

void init_sd(){
  // Start the MicroSD card
  // DEBUG_PRINTLN("Mounting MicroSD Card");
  if (!SD_MMC.begin("/sdcard", true)) {
    // DEBUG_PRINTLN("MicroSD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    // DEBUG_PRINTLN("No MicroSD Card found");
    return;
  }
  // DEBUG_PRINTLN("MicroSD card mount success!");
}