#ifndef MENU_H
#define MENU_H

#include "GUI.h"
#include "SettingKeypad.h"
#include "SettingWifi.h"
#include "deviceServer.h"
#include "Action.h"
#include "Joystick.h"

enum MenuState { MAIN_MENU,
                 SUB_MENU,
                 SUB_MENU_LEVEL_2 };
MenuState menuState = MAIN_MENU;

int maxMenu = 6;
int mainMenuIndex = 1;
int subMenuIndex = 0;
int menuIndex = 0;

int subMenu2Index = 0;
const char** subMenu2Items = nullptr;
int subMenu2Size = 0;
void (*subMenu2Callback)(int) = nullptr;

bool soundOn = true;
bool isAPMode = false;

const int subMenuSize[] = { 6, 2, 4, 2, 2, 2 };

const char* menuWifi[] = { "Connect", "Scan", "Info", "AP Mode", "Speed", "RSSI" };
const char* menuClock[] = { "Clock", "Count" };
const char* menuSetup[] = { "Control", "Sound", "Reset", "About" };
const char* menuSD[] = { "Read SD", "Remove" };
const char* menuRF[] = { "Read ID", "Remove" };
const char* menuJS[] = { "Test", "Control" };

// Cập nhật hiển thị menu với cơ chế cuộn
void displayMenu(const char* menuItems[], int menuSize) {
  oled.fillRect(0, 13, 128, 52, BLACK);

  for (int i = 0; i < 3; i++) {
    int itemIndex = menuIndex + i;
    if (itemIndex >= menuSize) break;

    int y = 26 + i * 16;
    u8g2Fonts.setCursor(11, y);
    u8g2Fonts.println(menuItems[itemIndex]);
  }

  // Hiển thị mũi tên chỉ vào mục đang được chọn
  int arrowY = 26 + (subMenuIndex - menuIndex) * 16;
  drawBitmap(ArrowMenu, 0, arrowY - 10, 8, 12);

  oled.display();
}

void (*subMenu2DrawExtra)() = nullptr;
void displaySubMenuLevel2() {
  if (subMenu2Items == nullptr) return;
  oled.fillRect(0, 13, 128, 52, BLACK);

  for (int i = 0; i < subMenu2Size; i++) {
    int y = 26 + i * 16;
    u8g2Fonts.setCursor(11, y);
    u8g2Fonts.println(subMenu2Items[i]);
  }

  // Gọi vẽ bổ sung nếu có
  if (subMenu2DrawExtra) {
    subMenu2DrawExtra();
  }

  // Mũi tên
  int arrowY = 26 + subMenu2Index * 16;
  drawBitmap(ArrowMenu, 0, arrowY - 10, 8, 12);

  oled.display();
}

void openSubMenuLevel2(const char** items, int size, void (*callback)(int), void (*drawExtra)() = nullptr) {
  subMenu2Items = items;
  subMenu2Size = size;
  subMenu2Callback = callback;
  subMenu2DrawExtra = drawExtra;  // gán hàm vẽ thêm (nếu có)
  subMenu2Index = 0;
  menuState = SUB_MENU_LEVEL_2;
  displaySubMenuLevel2();
}

void displayMainMenu() {
  switch (mainMenuIndex) {
    case 1:
      displayArrow();
      displayWifi();
      break;
    case 2:
      displayArrow();
      displayClock();
      break;
    case 3:
      displayArrow();
      displaySetup();
      break;
    case 4:
      displayArrow();
      displaySD();
      break;
    case 5:
      displayArrow();
      displayRF();
      break;
    case 6:
      displayArrow();
      displayJS();
      break;
  }
}

void displaySubMenu() {
  switch (mainMenuIndex) {
    case 1:
      displayMenu(menuWifi, subMenuSize[0]);
      break;
    case 2:
      displayMenu(menuClock, subMenuSize[1]);
      break;
    case 3:
      displayMenu(menuSetup, subMenuSize[2]);
      break;
    case 4:
      displayMenu(menuSD, subMenuSize[3]);
      break;
    case 5:
      displayMenu(menuRF, subMenuSize[4]);
      break;
    case 6:
      displayMenu(menuJS, subMenuSize[5]);
      break;
  }
}

void readRFID() {
  startRFID();
  showWaitingRF();
  bool running = true;
  String lastUID = "";

  while (running) {
    char key = keypad.getKey();
    if (key == 'D') {
      menuState = SUB_MENU;
      displaySubMenu();
      running = false;
      break;
    }

    if (RFID.PICC_IsNewCardPresent() && RFID.PICC_ReadCardSerial()) {
      String uid = "";
      for (byte i = 0; i < RFID.uid.size; i++) {
        if (RFID.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(RFID.uid.uidByte[i], HEX);
      }
      uid.toUpperCase();

      // Kiểm tra trùng UID
      if (uid != "") {
        webSerial.println("🆔 UID: " + uid);
        showUID(uid);
        verifySound(100);
        delay(2000);
        showWaitingRF();
      }

      RFID.PICC_HaltA();       // Dừng giao tiếp thẻ
      RFID.PCD_StopCrypto1();  // Dừng mã hóa
    }

    delay(100);  // Không nên quá nhỏ
  }
  subMenuIndex = -1;
}

void controlJoystick() {
  bool running = true;
  int prevXLogic = -999;
  int prevYLogic = -999;
  bool prevButton = false;

  const int joyLeftMax = 4095;
  const int joyRightMax = 0;
  const int joyUpMax = 4095;
  const int joyDownMax = 0;
  const int joyCenterX = 2950;
  const int joyCenterY = 2950;

  const int DEAD_ZONE = 100;
  const int LOGIC_X_MAX = 30;
  const int LOGIC_Y_MAX = 20;

  const int centerX = 64;
  const int centerY = 38;
  const int radius = 20;

  while (running) {
    char key = keypad.getKey();
    if (key == 'D') {
      menuState = SUB_MENU;
      displaySubMenu();
      running = false;
      break;
    }

    JoystickData joy = readJoystick();
    float xRatio = 0.0;
    float yRatio = 0.0;

    if (abs(joy.x - joyCenterX) > DEAD_ZONE) {
      if (joy.x > joyCenterX) {
        xRatio = (float)(joy.x - joyCenterX - DEAD_ZONE) / (joyLeftMax - joyCenterX - DEAD_ZONE);
      } else {
        xRatio = (float)(joy.x - joyCenterX + DEAD_ZONE) / (joyCenterX - joyRightMax - DEAD_ZONE);
      }
    }

    if (abs(joy.y - joyCenterY) > DEAD_ZONE) {
      if (joy.y < joyCenterY) {
        yRatio = (float)(joyCenterY - joy.y - DEAD_ZONE) / (joyCenterY - joyDownMax - DEAD_ZONE);
      } else {
        yRatio = -(float)(joy.y - joyCenterY - DEAD_ZONE) / (joyUpMax - joyCenterY - DEAD_ZONE);
      }
    }

    // Chuẩn hóa vector nếu nằm ngoài đường tròn đơn vị
    float magnitude = sqrt(xRatio * xRatio + yRatio * yRatio);
    if (magnitude > 1.0) {
      xRatio /= magnitude;
      yRatio /= magnitude;
    }

    int xLogic = (int)(-xRatio * LOGIC_X_MAX);
    int yLogic = (int)(-yRatio * LOGIC_Y_MAX);

    // Vẽ lại nếu có thay đổi
    if (xLogic != prevXLogic || yLogic != prevYLogic || joy.button != prevButton) {
      oled.fillRect(0, 12, 128, 52, BLACK);

      // --------- Vẽ vòng tròn lớn cố định ở giữa ---------
      oled.drawCircle(centerX, centerY, radius, WHITE);

      // --------- Tính vị trí chấm nhỏ bên trong vòng tròn ---------
      int dotX = centerX - (int)(xRatio * radius);
      int dotY = centerY + (int)(yRatio * radius);

      if (joy.button) {
        oled.drawCircle(dotX, dotY, 2, WHITE);  // Nhấn thì đậm
      } else {
        oled.fillCircle(dotX, dotY, 2, WHITE);  // Không nhấn thì viền
      }

      // --------- Hiển thị thông tin X bên trái ---------
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(2, 30);
      oled.print("X");
      oled.setCursor(2, 40);
      oled.print(xLogic);

      // --------- Hiển thị thông tin Y bên phải ---------
      oled.setCursor(110, 30);
      oled.print("Y");
      oled.setCursor(110, 40);
      oled.print(yLogic);

      oled.display();

      prevXLogic = xLogic;
      prevYLogic = yLogic;
      prevButton = joy.button;
    }

    delay(50);
  }
}

void handleUARTMode() {
  bool running = true;
  webSerial.println("➡️ Log reading mode. Press 'D' on the keyboard to exit.");
  displayLogView();

  while (running) {
    // Nhận dữ liệu từ UART mặc định (Serial)
    if (Serial.available()) {
      String data = Serial.readStringUntil('\n');
      webSerial.println("Log: " + data);
    }

    // Kiểm tra phím từ keypad
    char key = keypad.getKey();
    if (key == 'D') {
      menuState = SUB_MENU;
      displaySubMenu();
      webSerial.println("➡️ Exited UART mode.");
      running = false;
      break;
    }

    delay(100);  // Delay nhỏ để kiểm tra phím nhanh hơn
  }
}

void executeSubMenuAction() {
  switch (mainMenuIndex) {
    case 1: // Wifi
      if (subMenuIndex == 0) {  // Nếu chọn "Connect"
        showWiFiList();
      }
      if (subMenuIndex == 1) {  // Nếu chọn "Scan"
        scanWiFi();
      }
      if (subMenuIndex == 2) {  // Nếu chọn "Info"
        displayInfoWifi();
      }
      if (subMenuIndex == 3) {  // Nếu chọn "AP Mode"
        static const char* apModeOptions[] = { "On", "Off" };
        openSubMenuLevel2(
          apModeOptions, 2, [](int idx) {
            if (idx == 0) {
              if (startAPMode()) {
                isAPMode = true;
                displayTrueAP();
              } else {
                displayFailAP();
              }
            } else if (idx == 1) {
              WiFi.softAPdisconnect(true);
              isAPMode = false;
              server.end();

              // Chuyển về STA mode và kết nối lại
              WiFi.mode(WIFI_STA);
              xTaskCreatePinnedToCore(WiFiAutoConnect, "WiFi Connect Task", 4096, NULL, 1, &WiFiTask, 0);
            }

            menuState = SUB_MENU;
            displaySubMenu();
          },
          []() {
            if (isAPMode) {
              oled.fillCircle(40, 23, 2, WHITE);
            } else {
              oled.fillCircle(40, 37, 2, WHITE);
            }
          });
      }
      if (subMenuIndex == 4) {  // Nếu chọn "Speed"
        displaySpeedWifi();
      }
      if (subMenuIndex == 5) {  // Nếu chọn "RSSI"
        initRSSIData();         // Khởi tạo nền đen
        bool running = true;    // Biến điều khiển vòng lặp
        while (running) {       // Vòng lặp hiển thị liên tục
          displayRSSIGraph();
          char key = keypad.getKey();
          if (key == 'D') {
            running = false;
            break;
          }
          delay(500);  // Giảm delay để nhận phím nhanh hơn
        }
      }
      break;
    case 2: // Clock
      if (subMenuIndex == 0) {
        displayTime();
      }
      break;
    case 3: // Setup
      if (subMenuIndex == 0) {
        handleUARTMode();
      }
      if (subMenuIndex == 1) {
        static const char* soundOptions[] = { "On", "Off" };
        openSubMenuLevel2(
          soundOptions, 2, [](int idx) {
            if (idx == 0) {
              soundOn = true;
              pinMode(17, OUTPUT);
            } else {
              soundOn = false;
              pinMode(17, INPUT);
              digitalWrite(17, LOW);
            }
          },
          []() {
            if (soundOn) {
              oled.fillCircle(40, 23, 2, WHITE);
            } else {
              oled.fillCircle(40, 37, 2, WHITE);
            }
          });
      }
      if (subMenuIndex == 2) {
        ESP.restart();
      }
      if (subMenuIndex == 3) {
        displayDeviceInfo();
      }
      break;
    case 4: // SD
      if (subMenuIndex == 0) {  // Read SD
        showFileList();
      }
      if (subMenuIndex == 1) {  // Remove
        SD.end();
        webSerial.println("✅ Removed SD Card");
        verifySound(300);
      }
      break;
    case 5: // RFID
      if (subMenuIndex == 0) {  // Read ID
        readRFID();
      }
      if (subMenuIndex == 1) {  // Remove
        endRFID();
      }
      break;
    case 6: // Joystick
      if (subMenuIndex == 0) {
        controlJoystick();
      }
      if (subMenuIndex == 1) {}
      break;
  }
}

void handleKeypad() {
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case 'A':  // Nút lên
        if (menuState == MAIN_MENU) {
          if (mainMenuIndex < maxMenu) mainMenuIndex++;
          else mainMenuIndex = 1;
          displayMainMenu();
        } else if (menuState == SUB_MENU) {
          if (subMenuIndex > 0) {
            subMenuIndex--;
          } else {
            subMenuIndex = subMenuSize[mainMenuIndex - 1] - 1;
            menuIndex = subMenuIndex - 2;
            if (menuIndex < 0) menuIndex = 0;
          }
          if (subMenuIndex < menuIndex) {
            menuIndex--;
          }
          displaySubMenu();
        } else if (menuState == SUB_MENU_LEVEL_2) {
          if (subMenu2Index > 0) subMenu2Index--;
          else subMenu2Index = subMenu2Size - 1;
          displaySubMenuLevel2();
        }
        break;

      case 'C':  // Nút xuống
        if (menuState == MAIN_MENU) {
          if (mainMenuIndex > 1) mainMenuIndex--;
          else mainMenuIndex = maxMenu;
          displayMainMenu();
        } else if (menuState == SUB_MENU) {
          if (subMenuIndex < subMenuSize[mainMenuIndex - 1] - 1) {
            subMenuIndex++;
          } else {
            subMenuIndex = 0;
            menuIndex = 0;
          }
          if (subMenuIndex >= menuIndex + 3) {
            menuIndex++;
          }
          displaySubMenu();
        } else if (menuState == SUB_MENU_LEVEL_2) {
          if (subMenu2Index < subMenu2Size - 1) subMenu2Index++;
          else subMenu2Index = 0;
          displaySubMenuLevel2();
        }
        break;

      case 'B':  // Nút chọn
        if (menuState == MAIN_MENU) {
          menuState = SUB_MENU;
          subMenuIndex = 0;
          menuIndex = 0;
          displaySubMenu();
        } else if (menuState == SUB_MENU) {
          executeSubMenuAction();
        } else if (menuState == SUB_MENU_LEVEL_2) {
          if (subMenu2Callback) subMenu2Callback(subMenu2Index);
          menuState = SUB_MENU;
          displaySubMenu();
        }
        break;

      case 'D':  // Nút thoát
        if (menuState == SUB_MENU_LEVEL_2) {
          menuState = SUB_MENU;
          displaySubMenu();
        } else if (menuState == SUB_MENU) {
          menuState = MAIN_MENU;
          displayMainMenu();
        }
        break;
    }
  }
}

#endif
