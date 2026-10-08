/******************************** ABOUT ********************************
 * TFT initialization and drawing functions for clearing display,
 * displaying retrieved card data and displaying error messages
 * when invalid card ID was detected.
***********************************************************************/

#ifndef TFT_H
#define TFT_H

#include <Arduino.h>
#include <string.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "types.h"

void display_sleep(TFT_eSPI *tft);
void display_wakeup(TFT_eSPI *tft);
void init_display(TFT_eSPI *tft);
void clear_display(TFT_eSPI *tft);
void display_card_data(TFT_eSPI *tft, Card card_versions[]);
void display_error_message(TFT_eSPI *tft, char detected_card_id[]);

#endif