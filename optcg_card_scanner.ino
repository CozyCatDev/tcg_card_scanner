/******************************** SOURCE ********************************
 * Author: Arthur Tiong Tze Yuan
 * Release date: dd-mm-yyyy
 ******************************** PROGRAM *******************************
 * This is a TCG card scanner project that performs letter detection of
 * captured card IDs (e.g. OPXX-YYY) with a template matching algorithm,
 * i.e. Zero-Mean Normalized Cross-Correlation without using any ML
 * or TinyML models. No model training or storage of parameters is
 * needed. Card data corresponding to the currently detected ID is
 * retrieved from a microSD card and displayed on a TFT display.
 ******************************** HARDWARE ******************************
 * 1) ESP32-CAM microcontroller w/ OV3660 camera module and microSD card
      slot
 * 2) SPI TFT display with ILI9341 driver
 * 3) LM358 IR wake-up detection circuit (refer to README.md)
 * 4) MicroSD card formatted as 4GB FAT
 ************************** FUNCTIONAL OVERVIEW *************************
 * 1)  Card is inserted into slot in 3D printed enclosure.
 * 2)  IR detection circuit sends active HIGH wake-up signal to MCU.
 * 3)  ESP32-CAM wakes up and initializes peripherals and variables.
 * 4)  ESP32-CAM takes picture of card ID.
 * 5)  ROI of card letters, integral images, squared integral images and
       Sobel maps are computed in one go.
 * 6)  ZNCC is computed by sliding templates stored in Flash memory across
       ROI. Integral images compute fast sums while Sobel maps reject
       image patches early where no edges are detected.
 * 7)  Union-find clustering based on Euclidean distance is used to
       cluster close detections. Detection with highest ZNCC score in
       each cluster is chosen while the rest are discarded.
 * 8)  Filtered letter detections are sorted in ascending order of
       x-coordinate to form a correct sequence.
 * 9)  Detected card ID is formatted according to known card formats.
 * 10) If card ID format is valid, card ID is binary searched in
       "card_index.bin" to obtain byte lookup addresses for all
       related card versions. Card data is then obtained by pointing
       file pointer at these lookup addresses in "card_data.bin". Both
       files are stored in the microSD card.
 * 11) Retrieved card data is displayed on the TFT display.
 * 12) ESP32-CAM enters deep-sleep.
 ********************************* TODO *********************************
 * 1) Fix blank TFT display bug after ESP32-CAM wakes up from deep-sleep.
 * 2) Use hardware timer to deep-sleep ESP32-CAM after certain amount of
      time where no card was detected.
 * 3) Swap to portable power supply, e.g. 3.7V LiPo battery with boost
      converter.
 ************************* FUTURE IMPROVEMENTS **************************
 * 1) Swap to ESP32-S3-CAM with more GPIO pins.
 * 2) Control TFT backlight with PWM GPIO.
 * 3) Sleep TFT and turn off backlight before ESP32-S3-CAM enters
      deep-sleep.
 * 4) Consider using two-pass CCL and ZNCC at letter centroid.
 * 5) Store card images in microSD card and display on TFT.
 * 6) Implement touchscreen functionality to toggle between different
      card versions.
 ***********************************************************************/

// #include <Arduino.h>
#include <stdint.h>
#include <math.h>
#include <WiFiClientSecure.h>
#include <string.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "config.h"
#include "types.h"
#include "helper.h"
#include "templates.h"
#include "camera.h"
#include "sd_card.h"
#include "zncc.h"
#include "prune_detections.h"
#include "read_card_data.h"
#ifdef USE_TFT
#include "tft.h"
#endif

// ROI buffer to store cropped image
static uint8_t roi_buf[ROI_SIZE];

#ifdef DOWNSAMPLE
static uint8_t downsampled_roi_buf[DOWNSAMPLED_ROI_SIZE];
#endif

#ifdef USE_INTEGRAL_IMAGES
// 32-bit integral images used to efficiently compute image patch mean and variance
uint32_t *integral_roi_buf;
uint32_t *sqr_integral_roi_buf;
#endif

#ifdef USE_SOBEL_REJECTION
// integral image of Sobel map to efficiently compute edge energy of current image patch
float *sobel_integral_buf;
#endif

camera_fb_t *fb;

#ifdef USE_TFT
TFT_eSPI tft = TFT_eSPI();
#endif

// stores data of all versions of currently detected card, e.g. OP10-024 version 1, 2, 3, ...
Card card_versions[MAX_CARD_VERSION_IDS];

// array of detections before/after union-find clustering and filtering
Detection detections[MAX_ALL_DETECTIONS];
Detection filtered_detections[MAX_ALL_DETECTIONS];

// card ID after union-find clustering and filtering, sorting by x-coordinate
char detected_card_id[MAX_ALL_DETECTIONS];

// card ID after formatting
char card_id[MAX_CARD_CHARACTERS] = {0};

// OPTCG API endpoint
// char server_path[MAX_URL_LENGTH] = "https://www.optcgapi.com/api/sets/card/";

/******************* INITIALIZATION *******************
 * Initialize peripherals in this order:
 * 1) Display
 * 2) Wake-up pin
 * 3) Camera
 * 4) MicroSD card
 * 5) PSRAM malloc for integral images
 ******************************************************/
void setup() {

  /* set WAKEUP_PIN as active HIGH wakeup pin. Since active HIGH is used, pulldown resistors are enabled */
  // esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 1);
  // rtc_gpio_pullup_dis(WAKEUP_PIN);
  // rtc_gpio_pulldown_en(WAKEUP_PIN);

  #ifdef USE_TFT
  init_display(&tft);
  // display_wakeup(&tft);
  // delay(100);
  #endif

  pinMode(WAKEUP_PIN, INPUT_PULLDOWN);

  // Disable brownout detector
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  DEBUG_BEGIN(115200);

  DEBUG_PRINT("Initializing the camera module...");
  init_camera();
  DEBUG_PRINTLN("Camera OK!");

  DEBUG_PRINT("Initializing the MicroSD card module... ");
  init_sd();

  // integral images stored in PSRAM as insufficient Flash memory
  #ifdef USE_INTEGRAL_IMAGES
  integral_roi_buf = (uint32_t*) ps_malloc(ROI_SIZE * sizeof(uint32_t));
  sqr_integral_roi_buf = (uint32_t*) ps_malloc(ROI_SIZE * sizeof(uint32_t));
  #endif
  #ifdef USE_SOBEL_REJECTION
  sobel_integral_buf = (float*) ps_malloc(ROI_SIZE * sizeof(float));
  #endif

  if(psramInit()){
    DEBUG_PRINTLN("PSRAM initialized.");
  }else{
    DEBUG_PRINTLN("PSRAM failed to initialize.");
  }
}

void loop() {
  /******************* WAIT FOR CARD *******************
   * Loop endlessly if no card detected
   * from IR detection circuit
   *****************************************************/
  #ifdef USE_TFT
  clear_display(&tft);
  tft.drawCentreString("Waiting for card...", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  #endif

  while(!digitalRead(WAKEUP_PIN));

  /******************* RESET VARIABLES *******************
   * Reset all arrays stored in RAM to zero.
   *******************************************************/
  memset(detections, 0, sizeof(detections));
  memset(filtered_detections, 0, sizeof(filtered_detections));
  memset(card_versions, 0, sizeof(card_versions));
  memset(detected_card_id, 0, sizeof(detected_card_id));
  memset(card_id, 0, sizeof(card_id));

  /******************* IMAGE CAPTURE *******************/
  #ifdef USE_TFT
  tft.drawCentreString("Capturing image", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  #endif
  unsigned long start_time = millis();
  take_photo(&fb);
  unsigned long end_time = millis();
  DEBUG_PRINTF("Time elapsed for capturing image: %.3f seconds\n", (end_time - start_time) / 1000.0f);
  #ifdef USE_TFT
  clear_display(&tft);
  #endif

  /******************* DOWNSAMPLE (MAX-POOL) *******************/
  #ifdef DOWNSAMPLE
  start_time = millis();
  downsample(fb->buf, fb->width, downsampled_roi_buf);
  end_time = millis();
  DEBUG_PRINTF("Time elapsed for downsampling image: %.3f seconds\n", (end_time - start_time) / 1000.0f);
  #endif

  /******************* PRE-PROCESS IMAGE BUFFER *******************
  * 1) Compute cropped ROI, roi_buf.
  * 2) Compute downsampled ROI, downsampled_roi_buf.
  * 3) Compute integral images, integral_roi_buf,
  *    sqr_integral_roi_buf for faster ZNCC algorithm.
  * 4) Compute Sobel integral image, sobel_integral_buf.
  *****************************************************************/
  #ifdef USE_TFT
  tft.drawCentreString("Cropping image", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  #endif
  start_time = millis();
  preprocess(
          #ifdef DOWNSAMPLE
          downsampled_roi_buf,
          DOWNSAMPLED_ROI_W,
          #else
          fb->buf,
          fb->width,
          #endif
          roi_buf
          #ifdef USE_INTEGRAL_IMAGES
          , integral_roi_buf,
          sqr_integral_roi_buf
          #endif
          #ifdef USE_SOBEL_REJECTION
          , sobel_integral_buf
          #endif
  );
  end_time = millis();
  DEBUG_PRINTF("Time elapsed for cropping ROI: %.3f seconds\n", (end_time - start_time) / 1000.0f);
  #ifdef USE_TFT
  clear_display(&tft);
  #endif

  /******************* ZNCC *******************
   * Zero-Mean Normalized Cross-Correlation
   * Optimizations:
   * 1) Downsampled ROI.
   * 2) Integral images.
   * 3) Early patch rejection with Sobel maps.
   ********************************************/
  #ifdef USE_TFT
  tft.drawCentreString("Computing ZNCC", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  #endif
  start_time = millis();
  int num_detections = zncc(roi_buf,
                            #ifdef USE_INTEGRAL_IMAGES
                            integral_roi_buf,
                            sqr_integral_roi_buf,
                            #endif
                            #ifdef USE_SOBEL_REJECTION
                            sobel_integral_buf,
                            #endif
                            templates,
                            detections);
  end_time = millis();
  DEBUG_PRINTF("Time elapsed for ZNCC: %.3f seconds\n", (end_time - start_time) / 1000.0f);
  #ifdef USE_TFT
  clear_display(&tft);
  #endif

  #ifdef USE_TFT
  tft.drawCentreString("Number of detections: " + String(num_detections), SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  delay(2000);
  clear_display(&tft);
  #endif

  DEBUG_PRINTF("Number of detections before pruning: %d\n", num_detections);

  /******************* UNION-FIND CLUSTERING & FILTERING *******************
   * Cluster detections in detections[] array with union-find algorithm
   * according to Euclidean distance and select detection with max ZNCC
   * score per cluster.
   *************************************************************************/
  #ifdef USE_TFT
  tft.drawCentreString("Filtering detections", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  #endif
  start_time = millis();
  int num_filtered_detections = cluster_and_select_max(detections, num_detections, filtered_detections);
  end_time = millis();
  DEBUG_PRINTF("Time elapsed for filtering detections: %.3f seconds\n", (end_time - start_time) / 1000.0f);
  #ifdef USE_TFT
  clear_display(&tft);
  #endif

  /******************* DETECTION SORTING BY X-COORDINATE *******************/
  #ifdef USE_TFT
  tft.drawCentreString("Sorting detections", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  #endif
  start_time = millis();
  sort_detections_by_x(filtered_detections, num_filtered_detections);
  end_time = millis();
  DEBUG_PRINTF("Time elapsed for sorting detections: %.3f seconds\n", (end_time - start_time) / 1000.0f);
  #ifdef USE_TFT
  clear_display(&tft);
  #endif

  for(int i = 0; i < num_filtered_detections; i++){
    detected_card_id[i] = filtered_detections[i].label;
  }
  detected_card_id[num_filtered_detections] = '\0';

  DEBUG_PRINTF("Number of detections after pruning: %d\n", num_filtered_detections);
  DEBUG_PRINTF("Card number before formatting: %s\n", detected_card_id);

  /******************* CARD ID FORMATTING *******************
   * Formats detected_card_id to obtain card_id, supported
   * formats are:
   * 1) OPXX-YYY (standard)
   * 2) STXX-YYY (starter)
   * 3) PRBXX-YYY (premium booster)
   * 4) EBXX-YYY (extra booster)
   * 5) PXX-YYY (promotional)
   **********************************************************/
  bool card_format_ok = format_card_id(detected_card_id, card_id);
  DEBUG_PRINTF("Card number after formatting: %s\n", card_id);

  if (card_format_ok) {
    /******************* FIND CARD DATA *******************
     * Binary search "card_index.bin" for card_id to obtain
     * byte address of all card versions in "card_data.bin".
     ******************************************************/
    #ifdef USE_TFT
    tft.drawCentreString("Finding card data", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
    #endif
    start_time = millis();
    read_all_card_versions(card_versions, card_id);
    end_time = millis();
    DEBUG_PRINTF("Time elapsed for finding card data: %.3f seconds\n", (end_time - start_time) / 1000.0f);
    #ifdef USE_TFT
    clear_display(&tft);
    #endif

    /******************* DISPLAY CARD DATA *******************/
    #ifdef USE_TFT
    display_card_data(&tft, card_versions);
    #endif
    #ifdef DEBUG
    DEBUG_PRINT("Detected: ");
    DEBUG_PRINT(card_id);
    DEBUG_PRINTLN("Card Text: ");
    DEBUG_PRINT(card_versions[0].card_text);
    DEBUG_PRINTLN("Market Price: ");
    DEBUG_PRINT(card_versions[0].card_market_price);
    DEBUG_PRINTLN();
    #endif
  }
  else {
    /******************* NO VALID CARD ID DETECTED *******************/
    #ifdef DEBUG
    DEBUG_PRINTLN("No such ID:");
    DEBUG_PRINT(detected_card_id);
    #endif
    #ifdef USE_TFT
    display_error_message(&tft, detected_card_id);
    #endif
  }
  
  /******************* RETURN CAMERA FRAME BUFFER & CLEAR DISPLAY *******************/
  esp_camera_fb_return(fb);

  #ifdef USE_TFT
  delay(3000);
  clear_display(&tft);
  #endif

  /******************* END *******************/

  // #ifdef USE_TFT
  // tft.drawCentreString("Going to sleep...", SCREEN_CENTER_X, SCREEN_CENTER_Y, FONT_SIZE);
  // delay(3000);
  // clear_display(&tft);
  // // sleep display
  // // display_sleep(&tft);
  // #endif

  // start deep sleep
  // esp_deep_sleep_start();

  // // Bind Wakeup to GPIO13 going LOW
  // // esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);

  // // DEBUG_PRINTLN("Entering sleep mode");
  // // delay(1000);

  // // Enter deep sleep mode
  // // esp_deep_sleep_start();
}
