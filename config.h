/******************************** ABOUT ********************************
 * Program configuration settings.
***********************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

// Best settings so far:
// #define DEBUG
// #define LIMIT_ZNCC_DETECTIONS
// // #define USE_FULL_RESOLUTION
// #define USE_INTEGRAL_IMAGES
// // #define REJECT_EARLY
// // #define OVERRIDE_DETECTIONS
// // #define DOWNSAMPLE
// #define USE_SOBEL_REJECTION
// // #define REMOVE_SMALL_CLUSTERS
// #define PRIORITIZE_LARGE_LABELS
// #define PAD_TEMPLATES
// #define USE_TFT
// #ifndef USE_TFT
// #define DEBUG_SERIAL
// #endif
// #define ZNCC_THRESHOLD 0.70f
// #define SOBEL_REJECTION_THRESHOLD 0.045f
// #define MAX_DETECTIONS_PER_LABEL 5

#define DEBUG
#define LIMIT_ZNCC_DETECTIONS
// #define USE_FULL_RESOLUTION
#define USE_INTEGRAL_IMAGES
// #define REJECT_EARLY
// #define OVERRIDE_DETECTIONS
// #define DOWNSAMPLE
#define USE_SOBEL_REJECTION
// #define CLUSTER_SAME_LABELS_ONLY
// #define REMOVE_SMALL_CLUSTERS
#define PRIORITIZE_LARGE_LABELS
#define PAD_TEMPLATES
// #define DOUBLE_PAD_TEMPLATES
#define USE_TFT
#ifndef USE_TFT
#define DEBUG_SERIAL
#endif

#define LED_PIN 33

// Camera pins for AI-Thinker ESP32-CAM (common board)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LCD pins
#define LCD_SDA           12
#define LCD_SCL           13

// LCD cols and rows
#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// wake-up pin
#define WAKEUP_PIN GPIO_NUM_12

// buzzer frequency in kHz and duration in ms
#define BUZZER_FREQUENCY 1000
#define BUZZER_DURATION 250

#define TEMPLATE_COUNT 14

#define FB_W 160
#define FB_H 120

// 0: no rotation (horizontal)
// 1: 90deg clockwise
// 2: 180deg clockwise
// 3: 270deg clockwise
#define SCREEN_ROTATION 3

#if SCREEN_ROTATION == 0 || SCREEN_ROTATION == 2
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#elif SCREEN_ROTATION == 1 || SCREEN_ROTATION == 3
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#endif
#define SCREEN_CENTER_X ((SCREEN_WIDTH) / 2)
#define SCREEN_CENTER_Y ((SCREEN_HEIGHT) / 2)
#define FONT_SIZE 2

// Configurable ROI (tune for your 3D-printed tunnel)
#define BASE_ROI_X 0
#define BASE_ROI_Y 30
#define BASE_ROI_W 150
#define BASE_ROI_H 45
#ifdef USE_FULL_RESOLUTION
#undef BASE_ROI_X
#undef BASE_ROI_Y
#undef BASE_ROI_W
#undef BASE_ROI_H
#define BASE_ROI_X 0
#define BASE_ROI_Y 0
#define BASE_ROI_W FB_W
#define BASE_ROI_H FB_H
#endif
#define BASE_ROI_SIZE ((BASE_ROI_W) * (BASE_ROI_H))

// downsampling always halves frame buffer dimensions
#define DOWNSAMPLED_ROI_X ((BASE_ROI_X) / 2)
#define DOWNSAMPLED_ROI_Y ((BASE_ROI_Y) / 2)
#define DOWNSAMPLED_ROI_W (FB_W / 2)
#define DOWNSAMPLED_ROI_H (FB_H / 2)
#define DOWNSAMPLED_ROI_SIZE ((DOWNSAMPLED_ROI_W) * (DOWNSAMPLED_ROI_H))

// if custom ROI defined, halve custom ROI, else define halved frame buffer as ROI
#define DOWNSAMPLED_CROPPED_ROI_W (BASE_ROI_W / 2)
#define DOWNSAMPLED_CROPPED_ROI_H (BASE_ROI_H / 2)
#define DOWNSAMPLED_CROPPED_ROI_SIZE ((DOWNSAMPLED_CROPPED_ROI_W) * (DOWNSAMPLED_CROPPED_ROI_H))

#ifdef DOWNSAMPLE
#define ROI_X DOWNSAMPLED_ROI_X
#define ROI_Y DOWNSAMPLED_ROI_Y
#define ROI_W DOWNSAMPLED_CROPPED_ROI_W
#define ROI_H DOWNSAMPLED_CROPPED_ROI_H
#define ROI_SIZE DOWNSAMPLED_CROPPED_ROI_SIZE
#else
#define ROI_X BASE_ROI_X
#define ROI_Y BASE_ROI_Y
#define ROI_W BASE_ROI_W
#define ROI_H BASE_ROI_H
#define ROI_SIZE BASE_ROI_SIZE
#endif

// ZNCC accept threshold (tune)
#define ZNCC_THRESHOLD 0.70f
#define SOBEL_REJECTION_THRESHOLD 0.045f
#define REJECTION_THRESHOLD 0.10f

// maximum number of best_scores, best_x, best_y per label
#define MAX_DETECTIONS_PER_LABEL 5

// flattened size of all detections
#define MAX_ALL_DETECTIONS (TEMPLATE_COUNT * MAX_DETECTIONS_PER_LABEL)

#define MIN_DETECTIONS_PER_CLUSTER 2

#define MAX_CLUSTERS 10

// cluster distance used for grouping detections that are close together
// if <= CLUSTER_DISTANCE (pixels), group point into cluster
// best point from cluster is point with highest ZNCC score
#define CLUSTER_DISTANCE 4
#define CLUSTER_DISTANCE_SQ (CLUSTER_DISTANCE * CLUSTER_DISTANCE)

// max number of card characters in card number
#define MAX_CARD_CHARACTERS 9

// buffer sizes for variable fields in card struct
#define MAX_CARD_SET_ID (MAX_CARD_CHARACTERS)
#define MAX_CARD_NAME 50
#define MAX_CARD_SET_NAME 50
#define MAX_CARD_TEXT 400
#define MAX_CARD_SUB_TYPES 50

// max card version IDs
#define MAX_CARD_VERSION_IDS 8

// card_index.bin memory allocations
#define BYTES_CARD_SET_ID 9
#define BYTES_CARD_ADDRESS 4
#define BYTES_CARD_ADDRESSES (MAX_CARD_VERSION_IDS * BYTES_CARD_ADDRESS)
#define BYTES_PER_INDEX (BYTES_CARD_SET_ID + BYTES_CARD_ADDRESSES)

// max URL length of OPTCG API endpoint
#define MAX_URL_LENGTH 80

// Buzzer pin
// #define BUZZER            12

#endif