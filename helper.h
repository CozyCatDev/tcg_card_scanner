/******************************** ABOUT ********************************
 * Helper functions for easily obtaining pixel values using pointer
 * arithmetic. Simple math macros and conditional serial printing is
 * also provided.
***********************************************************************/
#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>
#include "config.h"

#define OFFSET(W, X, Y) ((X) + (W) * (Y))
#define AT(ADDR, W, X, Y) ((ADDR) + (OFFSET(W, X, Y)))
#define TOP(ADDR, W) (AT(ADDR, W, 0, -1))
#define RIGHT(ADDR, W) (AT(ADDR, W, 1, 0))
#define BOTTOM(ADDR, W) (AT(ADDR, W, 0, 1))
#define LEFT(ADDR, W) (AT(ADDR, W, -1, 0))
#define TOP_RIGHT(ADDR, W) (AT(ADDR, W, 1, -1))
#define BOTTOM_RIGHT(ADDR, W) (AT(ADDR, W, 1, 1))
#define BOTTOM_LEFT(ADDR, W) (AT(ADDR, W, -1, 1))
#define TOP_LEFT(ADDR, W) (AT(ADDR, W, -1, -1))

#define SQR(N) ((N) * (N))

#ifdef DEBUG_SERIAL
  #define DEBUG_BEGIN(baud) Serial.begin(baud)
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_BEGIN(baud)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

#endif