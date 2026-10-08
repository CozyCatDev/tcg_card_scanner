#include "debug.h"

float max_value_array(float arr[], int size) {
  float maxValue = arr[0];
  for (int i = 1; i < size; i++) {
    maxValue = max(maxValue, arr[i]);
  }
  return maxValue;
}

void print_array(float arr[], int size) {
  DEBUG_PRINT("[");
  for (int i = 0; i < size; i++) {
    if (i == size - 1) {
      DEBUG_PRINTF("%.3f]\n", arr[i]);
    } else {
      DEBUG_PRINTF("%.3f, ", arr[i]);
    }
  }
}