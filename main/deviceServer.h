#ifndef SERVER_H
#define SERVER_H

#undef CLOSED

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ElegantOTA.h>
#include "WebSerialMonitor.h"
#include "WebReadSDCard.h"
#include "SettingWifi.h"
#include "Action.h"

#define CLOSED HIGH
#define FW_VERSION "1.0.0"

extern AsyncWebServer server(80);
extern WebSerialMonitor webSerial(&server);
extern WebReadSDCard webSD(&server, FW_VERSION);

#define MAX_WIFI_LIST 10
extern String wifiList[MAX_WIFI_LIST];
extern int wifiCount;
extern int wifiIndex;
extern int wifiMenuIndex;

void commandAction() {
  webSerial.cmdFromWeb([](const String& cmd) {
    if (cmd.equalsIgnoreCase("reboot")) {
      webSerial.println("♻️ ESP đang khởi động lại...");  // Thông báo hiện ra
      delay(1000);
      ESP.restart();  // Thực hiện
    } else if (cmd.startsWith("wifi")) {
      server.end();
      int firstSpace = cmd.indexOf('-');
      int secondSpace = cmd.indexOf('-', firstSpace + 1);

      if (secondSpace > 0) {
        String ssid = cmd.substring(firstSpace + 1, secondSpace);
        String pass = cmd.substring(secondSpace + 1);
        webSerial.println("📶 Đang kết nối đến WiFi: " + ssid);
        unsigned long startAttemptTime = millis();
        WiFi.begin(ssid, pass);
        Serial.print("Conneting to ");
        Serial.println(ssid);
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
          Serial.print(".");
          delay(500);
        }
        if (WiFi.status() == WL_CONNECTED) {
          verifySound(300);
          Serial.println("");
          Serial.println("WiFi connected");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          MDNS.begin("esp");
          connectNTP();
          webSerial.begin();
          commandAction();
          webSD.begin();
          server.begin();
          ElegantOTA.begin(&server);
        } else {
          failSound(100);
          webSerial.println("⚠️ Không thể kết nối");
        }
      } else {
        webSerial.println("⚠️ Cú pháp không hợp lệ. Dùng: wifi-<SSID>-<PASSWORD>");
      }
    } else if (cmd.equalsIgnoreCase("scan wifi")) {
      WiFi.mode(WIFI_STA);
      webSerial.println("Scanning wifi...");
      wifiCount = WiFi.scanNetworks();
      if (wifiCount == 0) {
        webSerial.println("No network found.");
      } else {
        for (int i = 0; i < wifiCount && i < MAX_WIFI_LIST; i++) {
          wifiList[i] = WiFi.SSID(i);  // Lưu SSID
          char buffer[100];
          snprintf(buffer, sizeof(buffer), "%d. %s", i + 1, wifiList[i].c_str());
          webSerial.println(buffer);
        }
      }
      wifiIndex = 0;
      wifiMenuIndex = 0;
    }
  });
}
#endif
