#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "types.h"
#include "helper.h"

extern const char *ssid;
extern const char *password;

void init_wifi();

#endif