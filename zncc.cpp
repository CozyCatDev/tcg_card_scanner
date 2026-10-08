#include "zncc.h"

// void zncc(
//   const uint8_t *roi,
//   const uint8_t *tpl_data, int tpl_w, int tpl_h,
//   TemplateScore *out) {
//   // float best_scores[MAX_DETECTIONS_PER_LABEL];
//   // uint8_t best_x[MAX_DETECTIONS_PER_LABEL];
//   // uint8_t best_y[MAX_DETECTIONS_PER_LABEL];
//   int score_index = 0;

//   const int tpl_size = tpl_w * tpl_h;

//   // --- template mean ---
//   int tpl_sum = 0;
//   for (int i = 0; i < tpl_size; i++) {
//     tpl_sum += tpl_data[i];
//   }
//   float tpl_mean = (float)tpl_sum / tpl_size;

//   // --- template variance ---
//   float tpl_var = 0.0f;
//   for (int i = 0; i < tpl_size; i++) {
//     float t = (float)tpl_data[i] - tpl_mean;
//     tpl_var += t * t;
//   }
//   if (tpl_var < 1e-12f) tpl_var = 1e-12f;

//   // --- slide window ---
//   // resultant image after ZNCC has dimensions of (ROI_W - tpl_w + 1) x (ROI_H - tpl_h + 1)
//   // not ROI_H - tpl_h + 1 or ROI_W - tpl_w + 1 because start from zero index
//   // y, x defines template offset from top left of image
//   for (int y = 0; y <= ROI_H - tpl_h; y++) {
//     for (int x = 0; x <= ROI_W - tpl_w; x++) {
//       if (score_index >= MAX_DETECTIONS_PER_LABEL) break;
//       float img_sum = 0.0f;
//       // j, i defines index within template
//       for (int j = 0; j < tpl_h; j++) {
//         for (int i = 0; i < tpl_w; i++) {
//           img_sum += roi[(y + j) * ROI_W + (x + i)];
//         }
//       }
//       float img_mean = (float)img_sum / tpl_size;

//       float num = 0.0f;
//       float img_var = 0.0f;
//       int k = 0;

//       for (int j = 0; j < tpl_h; j++) {
//         for (int i = 0; i < tpl_w; i++, k++) {
//           // force to zero mean for image and template
//           float I = (float)roi[(y + j) * ROI_W + (x + i)] - img_mean;
//           float T = (float)tpl_data[k] - tpl_mean;
//           // compute dot product between zero mean image and template
//           num += I * T;
//           // compute image var
//           img_var += I * I;
//         }
//       }

//       float denom = sqrtf(img_var * tpl_var);
//       if (denom < 1e-12f) continue;

//       float score = num / denom;
//       if (score > ZNCC_THRESHOLD) {
//         out->best_scores[score_index] = score;
//         out->best_x[score_index] = x;
//         out->best_y[score_index] = y;
//         score_index++;
//       }
//     }
//   }

//   // out->best_scores = best_scores;
//   // out->best_x = best_x;
//   // out->best_y = best_y;
// }

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
  Detection *detections) {
  int num_detections = 0;
  // float best_scores[MAX_DETECTIONS_PER_LABEL];
  // uint8_t best_x[MAX_DETECTIONS_PER_LABEL];
  // uint8_t best_y[MAX_DETECTIONS_PER_LABEL];
  for(int i = 0; i < TEMPLATE_COUNT; i++){
    int num_detections_per_label = 0;
    const int tpl_size = tpl[i].w * tpl[i].h;
    bool reject = false;
    // resultant image after ZNCC has dimensions of (ROI_W - tpl[i].w + 1) x (ROI_H - tpl_h + 1)
    for (int roi_y = 0; (roi_y <= ROI_H - tpl[i].h && !reject); roi_y++) {
      for (int roi_x = 0; roi_x <= ROI_W - tpl[i].w; roi_x++) {
        #ifdef USE_SOBEL_REJECTION
        // get four corners of Sobel integral image corresponding to current image patch
        float sobel_int_A = roi_y > 0 ? *AT(sobel_int_roi, ROI_W, roi_x + tpl[i].w - 1, roi_y - 1) : 0;
        float sobel_int_C = roi_x > 0 ? *AT(sobel_int_roi, ROI_W, roi_x - 1, roi_y + tpl[i].h - 1) : 0;
        float sobel_int_B = (roi_x > 0 && roi_y > 0) ? *AT(sobel_int_roi, ROI_W, roi_x - 1, roi_y - 1) : 0;
        float sobel_int_D = *AT(sobel_int_roi, ROI_W, roi_x + tpl[i].w - 1, roi_y + tpl[i].h - 1);

        // sum of Sobel edge energy within current image patch can be computed from the four corners
        float sobel_sum = sobel_int_D - sobel_int_A - sobel_int_C + sobel_int_B;

        // normalize by dividing against (number of pixels in template x maximum Sobel value)
        float sobel_mean = sobel_sum / (tpl_size * 2040.0f);

        // DEBUG_PRINTLN();
        // DEBUG_PRINTF("Sobel mean: %.3f", sobel_mean);

        if(sobel_mean < SOBEL_REJECTION_THRESHOLD) continue;
        #endif

        #ifdef LIMIT_ZNCC_DETECTIONS
        if (num_detections_per_label >= MAX_DETECTIONS_PER_LABEL){
          reject = true;
          break;
        }
        #endif

        float img_sum = 0.0f;
        // compute image sum for ROI
        #ifdef USE_INTEGRAL_IMAGES
        // [B, ..., A]
        // [., ..., .]
        // [C, ..., D]
        uint32_t int_A = roi_y > 0 ? *AT(int_roi, ROI_W, roi_x + tpl[i].w - 1, roi_y - 1) : 0;
        uint32_t int_C = roi_x > 0 ? *AT(int_roi, ROI_W, roi_x - 1, roi_y + tpl[i].h - 1) : 0;
        uint32_t int_B = (roi_x > 0 && roi_y > 0) ? *AT(int_roi, ROI_W, roi_x - 1, roi_y - 1) : 0;
        uint32_t int_D = *AT(int_roi, ROI_W, roi_x + tpl[i].w - 1, roi_y + tpl[i].h - 1);

        uint32_t sqr_int_A = roi_y > 0 ? *AT(sqr_int_roi, ROI_W, roi_x + tpl[i].w - 1, roi_y - 1) : 0;
        uint32_t sqr_int_C = roi_x > 0 ? *AT(sqr_int_roi, ROI_W, roi_x - 1, roi_y + tpl[i].h - 1) : 0;
        uint32_t sqr_int_B = (roi_x > 0 && roi_y > 0) ? *AT(sqr_int_roi, ROI_W, roi_x - 1, roi_y - 1) : 0;
        uint32_t sqr_int_D = *AT(sqr_int_roi, ROI_W, roi_x + tpl[i].w - 1, roi_y + tpl[i].h - 1);

        img_sum = int_D - int_A - int_C + int_B;
        float sqr_img_sum = sqr_int_D - sqr_int_A - sqr_int_C + sqr_int_B;

        float img_var = sqr_img_sum - tpl_size * SQR(img_mean);
        #else
        // compute sum of current image patch by summing every pixel (slower)
        for (int tpl_y = 0; tpl_y < tpl[i].h; tpl_y++) {
          for (int tpl_x = 0; tpl_x < tpl[i].w; tpl_x++) {
            img_sum += *AT(roi, ROI_W, roi_x + tpl_x, roi_y + tpl_y);
          }
        }
        #endif

        float img_mean = img_sum / tpl_size;

        #ifdef REJECT_EARLY
        float mean_err = fminf(
          fabsf(img_mean - tpl[i].mean), // low if white letters detected
          fabsf((255.0f - img_mean) - tpl[i].mean) // low if black letters detected
        );
        if(mean_err > (REJECTION_THRESHOLD * 255.0f)) continue;
        #endif

        float num = 0.0f;
        #ifndef USE_INTEGRAL_IMAGES
        float img_var = 0.0f;
        #endif
        int k = 0;

        // calculate ZNCC score per output pixel
        for (int tpl_y = 0; tpl_y < tpl[i].h; tpl_y++) {
          for (int tpl_x = 0; tpl_x < tpl[i].w; tpl_x++, k++) {
            // force to zero mean for image and template
            float I = (float)*AT(roi, ROI_W, roi_x + tpl_x, roi_y + tpl_y) - img_mean;
            float T = (float)tpl[i].data[k] - tpl[i].mean;
            // compute dot product between zero mean image and template
            num += I * T;
            #ifndef USE_INTEGRAL_IMAGES
            img_var += I * I;
            #endif
          }
        }

        float denom = sqrtf(img_var * tpl[i].var);
        if (denom < 1e-12f) continue;


        float score = num / denom;
        // DEBUG_PRINTF("ZNCC score for label %c @ (%d, %d): %.4f\n", tpl[i].label, roi_x, roi_y, score);
        if (fabsf(score) > ZNCC_THRESHOLD) {
          DEBUG_PRINTF("ZNCC score for label %c @ (%d, %d): %.4f\n", tpl[i].label, roi_x, roi_y, score);
          detections[num_detections].label = tpl[i].label;
          detections[num_detections].x = roi_x;
          detections[num_detections].y = roi_y;
          detections[num_detections].score = score;
          num_detections++;
          num_detections_per_label++;
        }
      }
    }
  }


  return num_detections;

  // out->best_scores = best_scores;
  // out->best_x = best_x;
  // out->best_y = best_y;
}