/******************************** ABOUT ********************************
 * Camera initialization and buffer pre-processing functions.
***********************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include <stdint.h>
#include <math.h>

#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include "config.h"
#include "types.h"
#include "helper.h"

/**
 * @brief ESP32-CAM camera initialization for OV3660 camera.
 * @details Configuration for camera GPIO, clock frequency, image format, frame buffer, post-processing techniques and flash LED.
 */
void init_camera();

#ifdef DOWNSAMPLE
/**
 * @brief Average-pool downsampling.
 * @param[in] fb Frame buffer pointer.
 * @param[in] fb_w Frame buffer width.
 * @param[out] roi_buf Downsampled ROI buffer.
 */
void downsample(uint8_t *fb, size_t fb_w, uint8_t *roi_buf);
#endif

/**
 * @brief Crops ROI buffer from frame buffer, fb and computes integral/square integral images and sobel maps.
 * @param[in] fb Frame buffer pointer.
 * @param[in] fb_w Frame buffer width.
 * @param[out] roi_buf Cropped ROI buffer.
 * @param[out] integral_roi_buf Integral image of cropped ROI buffer.
 * @param[out] sqr_integral_roi_buf Squared integral image of cropped ROI buffer.
 * @param[out] sobel_integral_buf Integral image of computed Sobel map.
 */
void preprocess(uint8_t *fb,
            size_t fb_w,
            uint8_t *roi_buf
            #ifdef USE_INTEGRAL_IMAGES
            , uint32_t *integral_roi_buf,
            uint32_t *sqr_integral_roi_buf
            #endif
            #ifdef USE_SOBEL_REJECTION
            , float *sobel_integral_buf
            #endif
);

/**
 * @brief Captures a new photo by turning on flash LED, capturing image into the frame buffer, fb, then turning off the flash LED.
 * @param[in] fb Frame buffer pointer.
 */
void take_photo(camera_fb_t **fb);

#endif