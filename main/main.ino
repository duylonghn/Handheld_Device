#include "SettingKeypad.h"
#include "Action.h"
#include "GUI.h"
#include "SettingWifi.h"
#include "Menu.h"

String wifiList[MAX_WIFI_LIST];

TaskHandle_t Task_Battery;
TaskHandle_t Task_WiFi;

void updateWiFiStatus(void* pvParameters) {
  while (1) {
    displayWifiStatus();                    // Cập nhật trạng thái WiFi
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Cập nhật mỗi 5 giây
  }
}

void updateBattery(void* pvParameters) {
  while (1) {
    int percent = batteryPercentage();      // Đọc % pin
    displayBattery(percent);                // Cập nhật màn hình
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Cập nhật mỗi 5 giây
  }
}

void setup() {
  Serial.begin(115200);
  checkOled();
  xTaskCreatePinnedToCore(WiFiAutoConnect, "WiFi Connect Task", 4096, NULL, 1, &WiFiTask, 0);
  waitScreen();
  delay(5000);

  pinMode(17, OUTPUT);
  initJoystick();

  oled.fillRect(0, 0, 128, 64, BLACK);
  displayWifiStatus();
  displayHeader();
  displayArrow();
  displayMainMenu();

  // Tạo Task cho WiFi và Pin
  xTaskCreatePinnedToCore(updateWiFiStatus, "WiFi Status", 2048, NULL, 1, &Task_WiFi, 0);
  xTaskCreatePinnedToCore(updateBattery, "Battery Monitor", 2048, NULL, 1, &Task_Battery, 0);
}

void loop() {
  handleKeypad();  // Xử lý bàn phím liên tục
  ElegantOTA.loop();
}
