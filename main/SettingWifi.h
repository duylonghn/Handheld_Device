#ifndef SETTING_WIFI_H
#define SETTING_WIFI_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <ESP32Ping.h>
#include "SettingTime.h"
#include "SettingKeypad.h"
#include "GUI.h"
#include "ReadSD.h"
#include "Action.h"

#include "deviceServer.h"

WiFiMulti wifiMulti;
extern Keypad keypad;
extern String getTextInput(int x, int y);

extern Adafruit_SH1106G oled;
#define WHITE SH110X_WHITE
#define BLACK SH110X_BLACK
extern void updateDisplayText(int x, int y, const String& inputText);
extern void drawBorder(int x, int y);
extern void displayConnectWifi(const char* ssid);
extern void displayConnectedWifi();
extern void displayFailConnect(const char* ssid);
extern void displayWifiStatus();

#define MAX_WIFI_LIST 10  // Số mạng tối đa hiển thị
extern String wifiList[MAX_WIFI_LIST];
int wifiCount = 0;
int wifiIndex = 0;
int wifiMenuIndex = 0;

bool WifiStatus = false;

IPAddress local_ip(192, 168, 1, 4);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Tên WiFi và mật khẩu
const char* ssidAP = "ESP32-AP";
const char* passAP = "12341234";

void connectWifi(const char* ssid, const char* pass) {
  if (WiFi.status() == WL_CONNECTED) {
    server.end();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  displayConnectWifi(ssid);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    verifySound(300);
    WifiStatus = true;
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    MDNS.begin("esp");
    displayConnectedWifi();
    connectNTP();
    webSerial.begin();
    commandAction();
    webSD.begin();
    server.begin();
    ElegantOTA.begin(&server);
  } else {
    WifiStatus = false;
    displayFailConnect(ssid);
  }
};

TaskHandle_t WiFiTask;
volatile bool stopAutoConnect = false;

void WiFiAutoConnect(void* param) {
  wifiMulti.addAP("Duy Long", "Khong@biet");
  wifiMulti.addAP("DuyLong", "12341234");
  wifiMulti.addAP("Wifi T1", "12341234");

  int retry = 0;
  while (wifiMulti.run() != WL_CONNECTED && retry++ < 10) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    WifiStatus = true;
    Serial.println("✅ WiFi connected");
    Serial.print("📡 IP address: ");
    Serial.println(WiFi.localIP());

    MDNS.begin("esp");
    displayWifiStatus();
    connectNTP();
    webSerial.begin();
    commandAction();
    webSD.begin();
    server.begin();
    ElegantOTA.begin(&server);
  } else {
    WifiStatus = false;
    Serial.println("❌ Không kết nối được WiFi nào.");
  }

  vTaskDelete(NULL);
}

bool startAPMode() {
  server.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  if (!WiFi.softAP(ssidAP, passAP)) {
    return false;
  }

  if (!MDNS.begin("esp")) {
    return false;
  }
  webSerial.begin();
  commandAction();
  webSD.begin();
  server.begin();
  return true;
}

void scanWiFi() {
  WiFi.mode(WIFI_STA);
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(2, 14);
  oled.print("Scanning wifi ...");
  oled.display();
  Serial.println("Scanning wifi...");
  wifiCount = WiFi.scanNetworks();
  if (wifiCount == 0) {
    Serial.println("No network found.");
    oled.fillRect(0, 13, 128, 52, BLACK);
    oled.setCursor(0, 20);
    oled.print("No network found.");
    oled.display();
  } else {
    Serial.printf("%d networks found:\n", wifiCount);
    for (int i = 0; i < wifiCount && i < MAX_WIFI_LIST; i++) {
      wifiList[i] = WiFi.SSID(i);  // Lưu SSID
      Serial.printf("%d. %s\n", i + 1, wifiList[i].c_str());
    }
    oled.fillRect(0, 13, 128, 64, BLACK);
    oled.setCursor(2, 14);
    oled.print("Wifi scan done!");
    oled.display();
  }
  wifiIndex = 0;
  wifiMenuIndex = 0;
}

// Hiển thị danh sách WiFi, chọn và nhập mật khẩu
void showWiFiList();
void enterWiFiPassword(String ssid) {
  oled.fillRect(0, 13, 128, 52, BLACK);
  oled.setTextColor(WHITE);
  oled.setCursor(2, 14);
  oled.print("SSID: ");
  oled.print(ssid);
  oled.setCursor(2, 26);
  oled.print("Password:");
  drawBorder(0, 38);
  oled.display();

  String password = getTextInput(4, 41);
  if (password == "EXIT") {
    showWiFiList();  // Quay lại danh sách WiFi nếu nhấn D
    return;
  }

  updateDisplayText(4, 41, password);
  connectWifi(ssid.c_str(), password.c_str());
}

void showWiFiList() {
  if (wifiCount == 0) {
    Serial.println("Không có mạng WiFi nào đã quét.");
    oled.fillRect(0, 13, 128, 52, BLACK);
    oled.setCursor(0, 20);
    oled.print("No network found.");
    oled.display();
    delay(2000);
    return;
  }

  int selectedWiFi = 0;
  int wifiMenuIndex = 0;
  bool confirmed = false;

  while (!confirmed) {
    oled.fillRect(0, 13, 128, 52, BLACK);

    // Hiển thị tối đa 3 mạng WiFi
    for (int i = 0; i < 3; i++) {
      int idx = wifiMenuIndex + i;
      if (idx >= wifiCount) break;

      oled.setCursor(0, 13 + i * 16);
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      if (idx == selectedWiFi) oled.print("> ");
      oled.print(wifiList[idx]);
    }
    oled.display();

    char key = keypad.getKey();
    if (key == 'A' && selectedWiFi > 0) {
      selectedWiFi--;
      if (selectedWiFi < wifiMenuIndex) wifiMenuIndex--;
    } else if (key == 'C' && selectedWiFi < wifiCount - 1) {
      selectedWiFi++;
      if (selectedWiFi >= wifiMenuIndex + 3) wifiMenuIndex++;
    } else if (key == 'B') {
      enterWiFiPassword(wifiList[selectedWiFi]);
      confirmed = true;
    } else if (key == 'D') {
      return;
    }
    delay(100);
  }
}


String getEncryptionType(wifi_auth_mode_t authMode) {
  switch (authMode) {
    case WIFI_AUTH_OPEN: return "Open";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
    default: return "Unknown";
  }
}

// URL file để đo tốc độ mạng
const char* downloadURL = "http://speed.hetzner.de/10MB.bin";
float testDownloadSpeed();
float testUploadSpeed();

// Hàm đo tốc độ tải xuống
float testDownloadSpeed() {
  HTTPClient http;
  http.begin(downloadURL);

  Serial.println("Bắt đầu đo tốc độ tải xuống...");
  unsigned long startTime = millis();
  int httpCode = http.GET();

  if (httpCode > 0) {
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[1024];  // Đọc dữ liệu theo từng block 1KB
    int totalBytes = 0;

    while (http.connected()) {
      int bytesRead = stream->readBytes(buffer, sizeof(buffer));
      if (bytesRead <= 0) break;
      totalBytes += bytesRead;
    }

    unsigned long endTime = millis();
    float timeTaken = (endTime - startTime) / 1000.0;  // Chuyển ms -> giây
    float speed = (totalBytes / 1024.0) / timeTaken;   // KB/s
    Serial.printf("Tốc độ tải xuống: %.2f KB/s\n", speed);

    http.end();
    return speed;
  }

  Serial.println("Lỗi khi tải dữ liệu.");
  http.end();
  return 0;
}

// Hàm đo tốc độ upload bằng cách gửi dữ liệu lên server
float testUploadSpeed() {
  WiFiClient client;
  const char* server = "httpbin.org";

  Serial.println("Bắt đầu đo tốc độ upload...");
  unsigned long startTime = millis();

  if (client.connect(server, 80)) {
    client.print("POST /post HTTP/1.1\r\n");
    client.print("Host: httpbin.org\r\n");
    client.print("Content-Length: 10240\r\n");  // 10KB dữ liệu giả
    client.print("Content-Type: application/octet-stream\r\n");
    client.print("\r\n");

    uint8_t buffer[1024] = { 0 };  // Dữ liệu giả 1KB
    for (int i = 0; i < 10; i++) {
      client.write(buffer, sizeof(buffer));
    }

    unsigned long endTime = millis();
    float timeTaken = (endTime - startTime) / 1000.0;  // Chuyển ms -> giây
    float speed = (10240 / 1024.0) / timeTaken;        // KB/s

    Serial.printf("Tốc độ upload: %.2f KB/s\n", speed);
    client.stop();
    return speed;
  }

  Serial.println("Lỗi khi gửi dữ liệu.");
  return 0;
}

#endif