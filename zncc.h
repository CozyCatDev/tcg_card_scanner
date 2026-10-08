/******************************** ABOUT ********************************
 * Zero-Mean Normalized Cross-Correlation function.
***********************************************************************/
#ifndef ZNCC_H
#define ZNCC_H

#include <Arduino.h>
#include <stdint.h>
#include <math.h>

#include "config.h"
#include "types.h"
#include "helper.h"

// void zncc(
//   const uint8_t *roi,
//   const uint8_t *tpl_data,
//   int tpl_w,
//   int tpl_h,
//   TemplateScore *out
// );

/**
 * @brief Zero-mean normalized cross-correlation.
 * @details Optimized ZNCC algorithm through fast computed sums using integral images and skipping computations using a Sobel map.
 * @param[in] roi ROI buffer (ROI_W * ROI_H).
 * @param[in] int_roi Integral image ROI buffer (ROI_W * ROI_H).
 * @param[in] sqr_int_roi Squared integral image ROI buffer (ROI_W * ROI_H).
 * @param[in] sobel_int_roi Sobel map of integral image ROI buffer (ROI_W * ROI_H).
 * @param[in] tpl Current template to compute ZNCC against.
 * @param[out] detections Array of detections where ZNCC >= ZNCC_THRESHOLD.
 * @return Number of detections where ZNCC >= ZNCC_THRESHOLD.
 */
int zncc(
  const uint8_t *roi,
  #ifdef USE_INTEGRAL_IMAGES
  uint32_t *int_roi,
  uint32_t *sqr_int_roi,
  #endif
  #ifdef USE_SOBEL_REJECTION
  float *sobel_int_roi,
  #endif
  const TemplateInfo *tpl,
  Detection *detections
);

#endif