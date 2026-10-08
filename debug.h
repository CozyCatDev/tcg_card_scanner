#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "types.h"
#include "helper.h"

float max_value_array(float arr[], int size);
void print_array(float arr[], int size);

#endif