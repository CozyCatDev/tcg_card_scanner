#include "tft.h"

void display_sleep(TFT_eSPI *tft){
  tft->writecommand(0x10);
  delay(5);
}

void display_wakeup(TFT_eSPI *tft){
  tft->writecommand(0x11);
  delay(120);
}

void init_display(TFT_eSPI *tft){
  tft->init();
  tft->setRotation(SCREEN_ROTATION);
  tft->fillScreen(TFT_BLACK);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
}

void clear_display(TFT_eSPI *tft){
  tft->fillScreen(TFT_BLACK);
}

void display_card_data(TFT_eSPI *tft, Card card_versions[]){
  clear_display(tft);
  int center_x = SCREEN_WIDTH / 2;
  int center_y = SCREEN_HEIGHT / 2;
  const String card_set_id_text = "Card ID: " + String(card_versions[0].card_set_id);
  const String card_name_text = "Card name: " + String(card_versions[0].card_name);
  const String card_price_text = "Card price: " + String(card_versions[0].card_market_price);
  tft->drawCentreString(card_set_id_text, center_x, center_y - 20, FONT_SIZE);
  tft->drawCentreString(card_name_text, center_x, center_y, FONT_SIZE);
  tft->drawCentreString(card_price_text, center_x, center_y + 20, FONT_SIZE);
}

void display_error_message(TFT_eSPI *tft, char detected_card_id[]){
  int center_x = SCREEN_WIDTH / 2;
  int center_y = SCREEN_HEIGHT / 2;
  const String error_message = "No such card ID: " + String(detected_card_id);
  tft->drawCentreString(error_message, center_x, center_y, FONT_SIZE);
}