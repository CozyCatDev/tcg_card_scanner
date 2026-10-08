#include "read_card_data.h"

const char* cardRarities[] = {
  "C",
  "UC",
  "R",
  "SR",
  "L",
  "SEC",
  "TR",
  "PR"
};

const char* cardColors[] = {
  "Red",
  "Blue",
  "Green",
  "Yellow",
  "Purple",
  "Black",
  "Blue Red",
  "Green Red",
  "Red Yellow",
  "Purple Red",
  "Red Black",
  "Blue Green",
  "Blue Yellow",
  "Blue Purple",
  "Blue Black",
  "Green Yellow",
  "Green Purple",
  "Green Black",
  "Purple Yellow",
  "Yellow Black",
  "Black Yellow",
  "Purple Black",
  "Rainbow"
};

const char* cardTypes[] = {
  "Leader",
  "Character",
  "Event",
  "Stage"
};

const char* cardAttributes[] = {
  "None",
  "Slash",
  "Strike",
  "Special",
  "Wisdom",
  "Ranged",
  "Slash Strike",
  "Slash Special",
  "Slash Wisdom",
  "Special Strike",
  "Strike Wisdom",
  "Ranged Strike",
  "Special Wisdom",
  "?"
};

bool format_card_id(char detected_labels[], char card_id[]) {

  int len = strlen(detected_labels) > 8 ? 8 : strlen(detected_labels);

  // Format: OPXXYYY  -> becomes OPXX-YYY (7 chars input)
  if ((detected_labels[0] == 'O' && detected_labels[1] == 'P') || (detected_labels[0] == 'S' && detected_labels[1] == 'T') || (detected_labels[0] == 'E' && detected_labels[1] == 'B')) {
    // Check digits
    for (int i = 2; i < 7; i++) {
      if (!isDigit(detected_labels[i])) return false;
    }

    // Build card_id
    card_id[0] = detected_labels[0];
    card_id[1] = detected_labels[1];
    card_id[2] = detected_labels[2];
    card_id[3] = detected_labels[3];
    card_id[4] = '-';
    card_id[5] = detected_labels[4];
    card_id[6] = detected_labels[5];
    card_id[7] = detected_labels[6];
    card_id[8] = '\0';

    return true;
  }

  // Format: PYYY  -> becomes P-YYY (4 chars input)
  if (detected_labels[0] == 'P') {

    for (int i = 1; i < 4; i++) {
      if (!isDigit(detected_labels[i])) return false;
    }

    card_id[0] = 'P';
    card_id[1] = '-';
    card_id[2] = detected_labels[1];
    card_id[3] = detected_labels[2];
    card_id[4] = detected_labels[3];
    card_id[5] = '\0';

    return true;
  }

  // Format: PRBXXYYY -> becomes PRBXX-YYY (9 chars input)
  if (detected_labels[0] == 'P' && detected_labels[1] == 'R' && detected_labels[2] == 'B') {

    // Check digits
    for (int i = 3; i < 8; i++) {
      if (!isDigit(detected_labels[i])) return false;
    }

    card_id[0] = 'P';
    card_id[1] = 'R';
    card_id[2] = 'B';
    card_id[3] = detected_labels[3];
    card_id[4] = detected_labels[4];
    card_id[5] = '-';
    card_id[6] = detected_labels[5];
    card_id[7] = detected_labels[6];
    card_id[8] = detected_labels[7];
    card_id[9] = '\0';

    return true;
  }

  return false;
}

bool read_variable_string(fs::File &file, char *buffer, uint16_t maxLen, uint8_t lengthFieldBytes)
{
  uint16_t lengthToRead = 0;

  if (lengthFieldBytes == 1){
      lengthToRead = file.read();
  }
  else{
    file.read((uint8_t*)&lengthToRead, 2);
  }

  if (lengthToRead == 0xFFFF) {  // None marker
      buffer[0] = '\0';
      return true;
  }

  if (lengthToRead >= maxLen)
      lengthToRead = maxLen - 1;

  file.read((uint8_t*)buffer, lengthToRead);
  buffer[lengthToRead] = '\0';

  return true;
}

void find_card_addresses(fs::File &file, char card_set_id[], uint32_t card_version_addresses[]){
  unsigned long file_size = file.size();
  uint16_t num_indices = file_size / BYTES_PER_INDEX;
  int16_t left = 0;
  int16_t right = num_indices - 1;
  char file_card_set_id[MAX_CARD_SET_ID];
  while(left <= right){
    uint16_t mid = (left + right) / 2;
    uint32_t mid_address = mid * BYTES_PER_INDEX;
    file.seek(mid_address);
    file.read((uint8_t*)file_card_set_id, BYTES_CARD_SET_ID);
    int cmp = memcmp(file_card_set_id, card_set_id, BYTES_CARD_SET_ID);
    if(cmp == 0){
      file.read((uint8_t*)card_version_addresses, BYTES_CARD_ADDRESSES);
      break;
    }else if(cmp < 0){
      left = mid + 1;
    }else{
      right = mid - 1;
    }
  }
}

bool read_all_card_versions(Card card_versions[], char card_set_id[])
{
  uint32_t card_version_addresses[MAX_CARD_VERSION_IDS];
  memset(card_version_addresses, 0, sizeof(card_version_addresses));
  fs::File card_index_file = SD_MMC.open("/card_index.bin", FILE_READ);
  if (!card_index_file) {
    // DEBUG_PRINTLN("Failed to open card_index.bin for reading.");
  }

  find_card_addresses(card_index_file, card_set_id, card_version_addresses);
  card_index_file.close();

  fs::File card_data_file = SD_MMC.open("/card_data.bin", FILE_READ);
  if (!card_data_file) {
    // DEBUG_PRINTLN("Failed to open card_data.bin for reading.");
  }

  for(int i = 0; i < MAX_CARD_VERSION_IDS; i++){
    uint32_t card_address = card_version_addresses[i];
    if (card_address == 0) continue;
    
    Card &card = card_versions[i];
    card_data_file.seek(card_address);
    read_variable_string(card_data_file, card.card_set_id, MAX_CARD_SET_ID, 1);

    card.card_version_id = card_data_file.read();

    card_data_file.read((uint8_t*)&card.card_market_price, 4);

    read_variable_string(card_data_file, card.card_name, MAX_CARD_NAME, 1);
    read_variable_string(card_data_file, card.card_set_name, MAX_CARD_SET_NAME, 1);
    read_variable_string(card_data_file, card.card_text, MAX_CARD_TEXT, 2);

    card.card_rarity = card_data_file.read();
    card.card_color = card_data_file.read();
    card.card_type = card_data_file.read();
    card.card_life = card_data_file.read();
    card.card_cost = card_data_file.read();
    card.card_power = card_data_file.read();

    read_variable_string(card_data_file, card.card_sub_types, MAX_CARD_SUB_TYPES, 1);

    card.card_counter_amount = card_data_file.read();
    card.card_attribute = card_data_file.read();
  }
  card_index_file.close();
  card_data_file.close();
  return true;
}