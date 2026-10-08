/******************************** ABOUT ********************************
 * Type definitions for TemplateInfo, Detection and Card.
***********************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#include "config.h"

typedef struct {
  char label;
  uint8_t w;
  uint8_t h;
  uint32_t sum;
  float mean;
  float var;
  const uint8_t *data;
} TemplateInfo;

// used to store best positions and scores of current label
// typedef struct {
//   char label;
//   uint8_t best_x[MAX_DETECTIONS_PER_LABEL];
//   uint8_t best_y[MAX_DETECTIONS_PER_LABEL];
//   float best_scores[MAX_DETECTIONS_PER_LABEL];
// } TemplateScore;

// used in cluster_and_select_max() to store a flat array of all detections obtained from template_scores
typedef struct {
  char label;
  uint8_t x;
  uint8_t y;
  float score;
} Detection;

typedef struct {
  char card_set_id[MAX_CARD_SET_ID];
  uint8_t card_version_id;
  float card_market_price;
  char card_name[MAX_CARD_NAME];
  char card_set_name[MAX_CARD_SET_NAME];
  char card_text[MAX_CARD_TEXT];
  uint8_t card_rarity;
  uint8_t card_color;
  uint8_t card_type;
  uint8_t card_life;
  uint8_t card_cost;
  uint8_t card_power;
  char card_sub_types[MAX_CARD_SUB_TYPES];
  uint16_t card_counter_amount;
  uint8_t card_attribute;
} Card;

#endif