/******************************** ABOUT ********************************
 * Functions for reading card data from microSD card and formatting card
 * IDs. Lookup arrays for card rarities, colors, types and attributes
 * are also included.
***********************************************************************/

#ifndef READ_CARD_DATA_H
#define READ_CARD_DATA_H

#include <stdint.h>

// MicroSD Libraries
#include "FS.h"
#include "SD_MMC.h"

#include "config.h"
#include "types.h"

extern const char* cardRarities[];
extern const char* cardColors[];
extern const char* cardTypes[];
extern const char* cardAttributes[];

bool format_card_id(char detected_labels[], char card_id[]);
bool read_variable_string(fs::File &file, char *buffer, uint16_t maxLen, uint8_t lengthFieldBytes);
void find_card_addresses(fs::File &file, char card_set_id[], uint32_t card_version_addresses[]);
bool read_all_card_versions(Card card_versions[], char card_set_id[]);

#endif