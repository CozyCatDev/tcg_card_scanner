/******************************** ABOUT ********************************
 * Union-find clustering and filtering.
***********************************************************************/

#ifndef PRUNE_DETECTIONS_H
#define PRUNE_DETECTIONS_H

#include <Arduino.h>
#include <stdint.h>
#include "config.h"
#include "types.h"
#include "templates.h"

extern int uf_parent[MAX_ALL_DETECTIONS];

/**
 * @brief Union-find initialization.
 * @param[in] n Number of initial roots.
 */
void uf_init(int n);

/**
 * @brief Returns the root of index a.
 * @details Performs path compression.
 * @param[in] a Index in uf_parent.
 * @return Root index of index a in uf_parent.
 */
int uf_find(int a);

/**
 * @brief Sets the parent of root of b to root of a.
 * @details Parent(Find(b)) = Find(a)
 * @param[in] a Find(a) becomes new parent of Find(b).
 * @param[in] b Find(a) becomes new parent of Find(b).
 */
void uf_union(int a, int b);
// int cluster_and_select_max(
//   TemplateScore template_scores[],
//   char out_labels[],
//   uint8_t out_x[],
//   uint8_t out_y[],
//   float out_score[]
// );

/**
 * @brief Uses union-find clustering to cluster detections and chooses detection with maximum ZNCC for each cluster.
 * @details Uses uf_init, uf_find, uf_union.
 * @param[in] detections Array of unfiltered detections produced by zncc().
 * @param[in] num_detections Length of detections array.
 * @param[out] filtered_detections Array of filtered detections by choosing maximum ZNCC score per cluster.
 * @return Length of filtered_detections array.
 */
int cluster_and_select_max(
  Detection *detections,
  int num_detections,
  Detection *filtered_detections
);
// void sort_detections_by_x(char labels[], uint8_t x[], uint8_t y[], float scores[], int num_detections);

/**
 * @brief Sorts letter detections in increasing order of x-coordinates, i.e. based on fd[i].x.
 * @details Uses bubble sort.
 * @param[in] num_filtered_detections Length of filtered_detections array.
 * @param[out] fd filtered_detections array. Modified such that detections are arranged in increasing order of x-coordinates.
 */
void sort_detections_by_x(Detection *fd, int num_filtered_detections);

#endif