#include "camera.h"

void init_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;

  // QQVGA resolution is 160 x 120 px
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // DEBUG_PRINTF("Camera init failed: 0x%x\n", err);
    while (1) delay(1000);
  }

  // camera quality adjustments
  sensor_t *s = esp_camera_sensor_get();

  // // BRIGHTNESS (-2 to 2)
  s->set_brightness(s, 0);
  // // CONTRAST (-2 to 2)
  s->set_contrast(s, 0);
  // // SATURATION (-2 to 2)
  s->set_saturation(s, 0);
  // // SPECIAL EFFECTS (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_special_effect(s, 0);
  // // WHITE BALANCE (0 = Disable , 1 = Enable)
  s->set_whitebal(s, 1);
  // // AWB GAIN (0 = Disable , 1 = Enable)
  s->set_awb_gain(s, 1);
  // // WB MODES (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_wb_mode(s, 0);
  // // EXPOSURE CONTROLS (0 = Disable , 1 = Enable)
  s->set_exposure_ctrl(s, 0);
  // // AEC2 (0 = Disable , 1 = Enable)
  s->set_aec2(s, 0);
  // // AE LEVELS (-2 to 2)
  // s->set_ae_level(s, 1);
  // // AEC VALUES (0 to 1200)
  s->set_aec_value(s, 400);
  // // GAIN CONTROLS (0 = Disable , 1 = Enable)
  s->set_gain_ctrl(s, 0);
  // // AGC GAIN (0 to 30)
  s->set_agc_gain(s, 0);
  // // GAIN CEILING (0 to 6)
  s->set_gainceiling(s, (gainceiling_t)0);
  // // BPC (0 = Disable , 1 = Enable)
  s->set_bpc(s, 1);
  // // WPC (0 = Disable , 1 = Enable)
  s->set_wpc(s, 1);
  // // RAW GMA (0 = Disable , 1 = Enable)
  s->set_raw_gma(s, 1);
  // // LENC (0 = Disable , 1 = Enable)
  s->set_lenc(s, 1);
  // // HORIZ MIRROR (0 = Disable , 1 = Enable)
  s->set_hmirror(s, 1);
  // // VERT FLIP (0 = Disable , 1 = Enable)
  s->set_vflip(s, 0);
  // // DCW (0 = Disable , 1 = Enable)
  s->set_dcw(s, 1);
  // // COLOR BAR PATTERN (0 = Disable , 1 = Enable)
  s->set_colorbar(s, 0);

  // DEBUG_PRINTLN("before turn off flash");

  // setup flash and turn it off initially
  rtc_gpio_hold_dis(GPIO_NUM_4);
  pinMode(GPIO_NUM_4, OUTPUT);
  analogWrite(GPIO_NUM_4, 0);
  // ledcAttach(FLASH_GPIO_NUM, LEDC_FREQ, LEDC_RES);
  // ledcWrite(FLASH_GPIO_NUM, 0);

  // DEBUG_PRINTLN("after turn off flash");
}

#ifdef DOWNSAMPLE
void downsample(uint8_t *fb, size_t fb_w, uint8_t *roi_buf){
  for(int y = 0; y < DOWNSAMPLED_ROI_H; ++y){
    for(int x = 0; x < DOWNSAMPLED_ROI_W; ++x){
      // 2 x 2 downsampling, so skip every 2 frames
      uint8_t *fb_curr = AT(fb, fb_w, x * 2, y * 2);
      uint8_t *roi_curr = AT(roi_buf, ROI_W, x, y);
      uint8_t right = *RIGHT(fb_curr, fb_w);
      uint8_t bottom = *BOTTOM(fb_curr, fb_w);
      uint8_t bottom_right = *BOTTOM_RIGHT(fb_curr, fb_w);

      // average pool (>> 2 equivalent to /4)
      *roi_curr = (*fb_curr + right + bottom + bottom_right) >> 2;
    }
  }
}
#endif

// ---------------- Crop ROI from frame buffer ----------------
// fb_w = fb->width, fb_h = fb->height
// void cropROI(uint8_t *fb, size_t fb_w, size_t fb_h, uint8_t *roi_buf) {
//   for (int r = 0; r < ROI_H; ++r) {
//     memcpy(AT(roi_buf, ROI_W, 0, r), AT(fb, fb_w, ROI_X, r + ROI_Y), ROI_W);
//   }
// }

void preprocess(uint8_t *fb, size_t fb_w, uint8_t *roi_buf
            #ifdef USE_INTEGRAL_IMAGES
            , uint32_t *integral_roi_buf, uint32_t *sqr_integral_roi_buf
            #endif
            #ifdef USE_SOBEL_REJECTION
            , float *sobel_integral_buf
            #endif
) {
  #ifdef USE_SOBEL_REJECTION
  // horizontal and vertical Sobel filters
  int8_t sh[3][3] = {{1,   2,  1},
                      {0,   0,  0},
                      {-1, -2, -1}};
  int8_t sv[3][3] = {{-1, 0, 1},
                      {-2, 0, 2},
                      {-1, 0, 1}};
  #endif
  for (int y = 0; y < ROI_H; ++y) {
    for(int x = 0; x < ROI_W; ++x){
      // get addresses of (x, y) in roi_buf and (ROI_X + x, ROI_Y + y) in fb
      uint8_t *roi_curr = AT(roi_buf, ROI_W, x, y);
      uint8_t *fb_curr = AT(fb, fb_w, ROI_X + x, ROI_Y + y);

      #ifdef USE_INTEGRAL_IMAGES
      // get addresses of (x, y) in integral images
      uint32_t *int_roi_curr = AT(integral_roi_buf, ROI_W, x, y);
      uint32_t *sqr_int_roi_curr = AT(sqr_integral_roi_buf, ROI_W, x, y);

      // get top, left and top left values of current pixel in integral image for computing current value
      uint32_t int_top = y > 0 ? *TOP(int_roi_curr, ROI_W) : 0;
      uint32_t int_left = x > 0 ? *LEFT(int_roi_curr, ROI_W) : 0;
      uint32_t int_top_left = (y > 0 && x > 0) ? *TOP_LEFT(int_roi_curr, ROI_W) : 0;

      // get top, left and top left values of current pixel in squared integral image for computing current value
      uint32_t sqr_int_top = y > 0 ? *TOP(sqr_int_roi_curr, ROI_W) : 0;
      uint32_t sqr_int_left = x > 0 ? *LEFT(sqr_int_roi_curr, ROI_W) : 0;
      uint32_t sqr_int_top_left = (y > 0 && x > 0) ? *TOP_LEFT(sqr_int_roi_curr, ROI_W) : 0;
      #endif

      #ifdef USE_SOBEL_REJECTION
      // get surrounding pixel values around current frame buffer pixel for computing convolution with Sobel filters
      uint8_t fb_top_left = (y > 0 && x > 0) ? *TOP_LEFT(fb_curr, fb_w) : 0;
      uint8_t fb_top = (y > 0) ? *TOP(fb_curr, fb_w) : 0;
      uint8_t fb_top_right = (y > 0 && x < ROI_W - 1) ? *TOP_RIGHT(fb_curr, fb_w) : 0;
      uint8_t fb_right = (x < ROI_W - 1) ? *RIGHT(fb_curr, fb_w) : 0;
      uint8_t fb_bottom_right = (y < ROI_H - 1 && x < ROI_W - 1) ? *BOTTOM_RIGHT(fb_curr, fb_w) : 0;
      uint8_t fb_bottom = (y < ROI_H - 1) ? *BOTTOM(fb_curr, fb_w) : 0;
      uint8_t fb_bottom_left = (y < ROI_H - 1 && x > 0) ? *BOTTOM_LEFT(fb_curr, fb_w) : 0;
      uint8_t fb_left = (x > 0) ? *LEFT(fb_curr, fb_w) : 0;
      
      // compute convolution with Sobel filters at current pixel value
      int32_t sobel_horizontal_sum = (fb_top_left * sh[0][0]) + (fb_top * sh[0][1]) + (fb_top_right * sh[0][2]) +
                                     (fb_right * sh[1][2]) + (fb_bottom_right * sh[2][2]) + (fb_bottom * sh[2][1]) +
                                     (fb_bottom_left * sh[2][0]) + (fb_left * sh[1][0]) + (*fb_curr * sh[1][1]);
      int32_t sobel_vertical_sum = (fb_top_left * sv[0][0]) + (fb_top * sv[0][1]) + (fb_top_right * sv[0][2]) +
                                   (fb_right * sv[1][2]) + (fb_bottom_right * sv[2][2]) + (fb_bottom * sv[2][1]) +
                                   (fb_bottom_left * sv[2][0]) + (fb_left * sv[1][0]) + (*fb_curr * sv[1][1]);
      float sobel_curr = fabsf((float)sobel_horizontal_sum) + fabsf((float)sobel_vertical_sum);

      // get address of current Sobel map output pixel
      float *sobel_int_curr = AT(sobel_integral_buf, ROI_W, x, y);

      // get top, left and top left of Sobel integral image to compute current Sobel integral image pixel
      float sobel_int_top = y > 0 ? *TOP(sobel_int_curr, ROI_W) : 0.0f;
      float sobel_int_left = x > 0 ? *LEFT(sobel_int_curr, ROI_W) : 0.0f;
      float sobel_int_top_left = (y > 0 && x > 0) ? *TOP_LEFT(sobel_int_curr, ROI_W) : 0.0f;

      // compute current Sobel integral image pixel value from top, left and top left pixel values
      *sobel_int_curr = sobel_curr + sobel_int_top + sobel_int_left - sobel_int_top_left;
      #endif

      // copy pixel from camera frame buffer into ROI
      *roi_curr = *fb_curr;

      #ifdef USE_INTEGRAL_IMAGES
      // compute current pixel values for integral images from top, left and top left pixel values
      *AT(integral_roi_buf, ROI_W, x, y) = *roi_curr + int_top + int_left - int_top_left;
      *AT(sqr_integral_roi_buf, ROI_W, x, y) = SQR(*roi_curr) + sqr_int_top + sqr_int_left - sqr_int_top_left;
      #endif
    }
  }
}

void take_photo(camera_fb_t **fb) {
  // LED flash
  analogWrite(GPIO_NUM_4, 127);
  delay(200);

  // flush old frames (underexposure bug)
  for (int i = 0; i < 2; i++) {
    camera_fb_t *tmp = esp_camera_fb_get();
    if (tmp) esp_camera_fb_return(tmp);
  }

  // get camera frame buffer
  *fb = esp_camera_fb_get();

  // turn off LED
  analogWrite(GPIO_NUM_4, 0);

  if (!(*fb)) {
    // DEBUG_PRINTLN("Camera capture failed");
  }

  // // Save picture to microSD card
  // fs::FS &fs = SD_MMC;
  // File file = fs.open(path.c_str(), FILE_WRITE);
  // if (!file) {
  //   DEBUG_PRINTLN("Failed to open file in write mode");
  // }
  // else {
  //   file.write(fb->buf, fb->len); // payload (image), payload length
  //   DEBUG_PRINTF("Saved file to path: %s\n", path.c_str());
  // }
  // // Close the file
  // file.close();
}

// // ---------------- Compute binary image (0/1) with threshold ----------------
// void binarize32(const uint8_t *src, uint8_t *dst, uint8_t thr) {
//   for (int i = 0; i < N_PIX; ++i) dst[i] = (src[i] < thr) ? 1 : 0;
// }

// // ---------------- Hamming distance (binary 0/1) ----------------
// int hamming32(const uint8_t *a, const uint8_t *b) {
//   int dif = 0;
//   for (int i = 0; i < N_PIX; ++i) if (a[i] != b[i]) ++dif;
//   return dif;
// }

// // ---------------- Load template grayscale from PROGMEM into tplBuf ----------------
// void loadTemplateProgmem(const uint8_t *tplProg, uint8_t *out) {
//   for (int i = 0; i < N_PIX; ++i) out[i] = pgm_read_byte(tplProg + i);
// }

// ---------------- Simple equal-slice segmentation ----------------
// void processROIandMatch(uint8_t *roi, int rw, int rh, char outLabels[6]) {
//   int digitW = rw / 5;
//   for (int d = 0; d < 5; ++d) {
//     // copy slice to contiguous sliceBuf
//     for (int r = 0; r < rh; ++r) {
//       memcpy(sliceBuf + r * digitW, roi + r * rw + d * digitW, digitW);
//     }
//     // resize (digitW x rh -> 32x32)
//     resizeNearest(sliceBuf, digitW, rh, digitW, sample32);
//     // match
//     LabelScore ls = matchSample(sample32);
//     if (ls.score >= ZNCC_ACCEPT) outLabels[d] = ls.label;
//     else outLabels[d] = '?';
//   }
//   outLabels[5] = '\0';
// }