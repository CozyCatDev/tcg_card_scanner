#include "wifi_manager.h"

// WiFi credentials
const char *ssid = "RobertT2608-Maxis Fibre";
const char *password = "LeicaQ343";

void init_wifi() {
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  DEBUG_PRINT("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
}

// String httpGETRequest(const char *serverName) {
//   WiFiClientSecure client;
//   client.setInsecure();  // Skip certificate validation (important for ESP32)

//   HTTPClient http;

//   http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

//   DEBUG_PRINT("Free heap before HTTPS: ");
//   DEBUG_PRINTLN(ESP.getFreeHeap());

//   if (!http.begin(client, serverName)) {
//     DEBUG_PRINTLN("HTTPS begin failed");
//     return "{}";
//   }

//   int httpResponseCode = http.GET();

//   String payload = "{}";

//   if (httpResponseCode > 0) {
//     DEBUG_PRINT("HTTP Response code: ");
//     DEBUG_PRINTLN(httpResponseCode);

//     payload = http.getString();
//   } else {
//     DEBUG_PRINT("Error code: ");
//     DEBUG_PRINTLN(httpResponseCode);
//   }

//   http.end();

//   DEBUG_PRINT("Free heap after HTTPS: ");
//   DEBUG_PRINTLN(ESP.getFreeHeap());

//   return payload;
// }

// void stopWiFi() {
//   WiFi.disconnect(true);
//   WiFi.mode(WIFI_OFF);
// }