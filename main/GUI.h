#ifndef GUI_H
#define GUI_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Bitmap.h"
#include "SettingWifi.h"
#include "SettingTime.h"
#include "RFID.h"

#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_flash.h>

extern Adafruit_SH1106G oled;

#define SCREEN_ADDRESS 0x3C  //SCL-22; SDA-21
Adafruit_SH1106G oled = Adafruit_SH1106G(128, 64, &Wire, -1);
#define WHITE SH110X_WHITE
#define BLACK SH110X_BLACK

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

#define RSSI_HISTORY 128             // Lưu 60 giá trị RSSI gần nhất
int rssiData[RSSI_HISTORY] = { 0 };  // Mảng lưu RSSI
int minRSSI = -90;                   // Ngưỡng tín hiệu yếu nhất
int maxRSSI = -50;                   // Ngưỡng tín hiệu mạnh nhất

#define INFO_LINES 10   // Tổng số dòng thông tin hiển thị
#define SCREEN_LINES 4  // Số dòng hiển thị trên màn hình
int infoIndex = 0;      // Vị trí dòng đầu tiên đang hiển thị

void checkOled() {
  // Kiểm tra kết nối oled
  if (!oled.begin()) {
    Serial.println(F("Display allocation failed"));
    for (;;)
      ;  // Dừng chương trình nếu không khởi tạo được màn hình oled
  }
  u8g2Fonts.begin(oled);
  u8g2Fonts.setFont(u8g2_font_ncenB08_tr);  // Chữ 8px, dễ đọc
};

// Hàm vẽ bitmap
void drawBitmap(const uint8_t* bitmap, int16_t x, int16_t y, int16_t width, int16_t height) {
  oled.drawBitmap(x, y, bitmap, width, height, WHITE);  // Vẽ bitmap tại tọa độ (x, y) với kích thước width x height
};

void waitScreen() {
  oled.clearDisplay();

  oled.setTextColor(1);
  oled.setCursor(19, 50);
  oled.print("Made by Duy Long");

  oled.setCursor(22, 7);
  oled.print("Handheld Device");

  oled.drawBitmap(9, 24, image_download_bits, 19, 16, 1);
  oled.drawBitmap(42, 24, image_download_1_bits, 16, 16, 1);
  oled.drawBitmap(73, 24, image_download_2_bits, 15, 16, 1);
  oled.drawBitmap(103, 24, image_download_3_bits, 16, 16, 1);
  oled.display();
}

void displayHeader() {
  drawBitmap(Header, 0, 0, 128, 12);
  drawBitmap(Bluetooth, 21, 1, 7, 9);
  drawBitmap(Battery, 107, 1, 20, 9);
}

void displayConnectWifi(const char* ssid) {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(1, 15);
  oled.print("Connecting to: ");
  oled.setCursor(1, 25);
  oled.print(ssid);
  oled.display();
}

void displayConnectedWifi() {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setCursor(0, 14);
  oled.println("Wifi Connected");
  oled.print("\nIP: ");
  oled.print(WiFi.localIP());
  oled.fillRect(32, 0, 14, 11, BLACK);
  drawBitmap(IconWifi, 32, 2, 14, 8);
  delay(3000);
  oled.display();
}

void displayFailConnect(const char* ssid) {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setCursor(0, 14);
  oled.println("Connection failed to:");
  oled.print(ssid);
  oled.fillRect(32, 0, 14, 11, BLACK);
  drawBitmap(IconNoWifi, 32, 1, 14, 10);
  delay(3000);
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.display();
}

void displayTrueAP() {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setCursor(0, 14);
  oled.println("AP mode started:");
  oled.setTextSize(2);
  oled.setCursor(0, 24);
  oled.println("ESP-AP");
  oled.fillRect(32, 0, 14, 11, BLACK);
  oled.display();
  delay(3000);
}

void displayFailAP() {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setCursor(0, 14);
  oled.println("AP mode can't start");
  oled.display();
  delay(3000);
}

void displayWifiStatus() {
  if (WifiStatus) {
    oled.fillRect(32, 0, 14, 11, BLACK);
    drawBitmap(IconWifi, 32, 2, 14, 8);
  } else {
    oled.fillRect(32, 0, 14, 11, BLACK);
    drawBitmap(IconNoWifi, 32, 0, 14, 10);
  }
}

void displayInfoWifi() {
  String wifiInfo[INFO_LINES];  // Mảng chứa thông tin WiFi

  // Khởi tạo các mục trống trước
  wifiInfo[0] = "SSID: Loading...";
  wifiInfo[1] = "IP: Loading...";
  wifiInfo[2] = "RSSI: Loading...";
  wifiInfo[3] = "Gateway: Loading...";
  wifiInfo[4] = "Subnet: Loading...";
  wifiInfo[5] = "MAC: Loading...";
  wifiInfo[6] = "Channel: Loading...";
  wifiInfo[7] = "Ping: Loading...";
  wifiInfo[8] = "Security: Loading...";

  bool inInfoScreen = true;
  infoIndex = 0;

  // Hiển thị các mục trước khi lấy dữ liệu
  while (inInfoScreen) {
    oled.fillRect(0, 13, 128, 52, BLACK);
    oled.setTextSize(1);
    oled.setTextColor(WHITE);

    for (int i = 0; i < SCREEN_LINES; i++) {
      int lineIndex = infoIndex + i;
      if (lineIndex >= INFO_LINES) break;
      oled.setCursor(2, 14 + i * 12);
      oled.print(wifiInfo[lineIndex]);
    }

    oled.display();

    // Chỉ lấy dữ liệu WiFi lần đầu tiên sau khi hiển thị giao diện
    static bool dataFetched = false;
    if (!dataFetched) {
      if (WiFi.status() == WL_CONNECTED) {
        wifiInfo[0] = "SSID: " + WiFi.SSID();
        wifiInfo[1] = "IP: " + WiFi.localIP().toString();
        wifiInfo[2] = "RSSI: " + String(WiFi.RSSI()) + " dBm";
        wifiInfo[3] = "Gateway: " + WiFi.gatewayIP().toString();
        wifiInfo[4] = "Subnet: " + WiFi.subnetMask().toString();
        wifiInfo[5] = "MAC: ";
        wifiInfo[6] = WiFi.BSSIDstr();
        wifiInfo[7] = "Channel: " + String(WiFi.channel());

        // Kiểm tra và lấy giá trị Ping
        int pingResult = Ping.ping(WiFi.gatewayIP()) ? Ping.averageTime() : -1;
        wifiInfo[8] = "Ping: " + String(pingResult) + " ms";
        wifiInfo[9] = "Security: " + getEncryptionType(WiFi.encryptionType(WiFi.channel()));
      } else {
        oled.fillRect(0, 13, 128, 52, BLACK);
        wifiInfo[0] = "Chua ket noi WiFi!";
        oled.display();
      }

      dataFetched = true;  // Chỉ lấy dữ liệu một lần
    }

    // Kiểm tra phím điều khiển
    char key = keypad.getKey();
    if (key) {
      if (key == 'A' && infoIndex > 0) {  // Cuộn lên
        infoIndex--;
      } else if (key == 'C' && infoIndex < INFO_LINES - SCREEN_LINES) {  // Cuộn xuống
        infoIndex++;
      } else if (key == 'D') {  // Quay lại menu chính
        inInfoScreen = false;
      }
    }
    delay(50);  // Giảm độ trễ nhận phím
  }
}

void displaySpeedWifi() {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  if (WiFi.status() == WL_CONNECTED) {
    oled.fillRect(64, 13, 2, 52, WHITE);
    oled.setTextSize(1);
    oled.setCursor(26, 14);
    oled.print("UP");
    oled.setCursor(85, 14);
    oled.print("DOWN");

    oled.setCursor(22, 52);
    oled.print("Kb/s");
    oled.setCursor(85, 52);
    oled.print("Kb/s");
    oled.display();

    oled.setTextSize(2);
    oled.setCursor(10, 27);
    oled.print("...");
    oled.setCursor(80, 27);
    oled.print("...");
    oled.display();

    float uploadSpeed = testUploadSpeed();
    oled.setTextSize(2);
    oled.setCursor(10, 30);
    oled.fillRect(0, 30, 63, 20, BLACK);
    oled.print(uploadSpeed);
    oled.display();

    float downloadSpeed = testDownloadSpeed();
    oled.setTextSize(2);
    oled.setCursor(75, 30);
    oled.fillRect(66, 30, 63, 20, BLACK);
    oled.print(downloadSpeed);
    oled.display();

    char key = keypad.getKey();
    if (key == 'D') {  // Quay lại menu chính
      return;
    }
  } else {
    oled.setCursor(2, 14);
    oled.print("Chua ket noi WiFi!");
    oled.display();
  }
}

void initRSSIData() {
  // Khởi tạo tất cả phần tử với minRSSI để bắt đầu với màn hình đen
  for (int i = 0; i < RSSI_HISTORY; i++) {
    rssiData[i] = minRSSI;
  }
}

void displayRSSIGraph() {
  oled.fillRect(0, 13, 128, 52, BLACK);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);

  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();  // Lấy cường độ tín hiệu WiFi hiện tại

    // Dịch toàn bộ dữ liệu cũ sang trái
    for (int i = 1; i < RSSI_HISTORY; i++) {
      rssiData[i - 1] = rssiData[i];
    }

    // Thêm dữ liệu RSSI mới vào cuối mảng
    rssiData[RSSI_HISTORY - 1] = rssi;

    // Hiển thị thông tin trên màn hình
    oled.setCursor(2, 14);
    oled.print(WiFi.SSID());

    oled.setCursor(2, 25);
    oled.print("RSSI: ");
    oled.print(rssi);
    oled.print(" dBm");

    // Vẽ đồ thị RSSI dưới cùng màn hình
    int graphHeight = 30;           // Chiều cao đồ thị (nửa dưới màn hình)
    int graphY = 64 - graphHeight;  // Đặt đồ thị ở nửa dưới màn hình

    for (int i = 0; i < RSSI_HISTORY; i++) {
      int height = map(rssiData[i], minRSSI, maxRSSI, 0, graphHeight);        // Chuyển RSSI thành chiều cao
      oled.drawFastVLine(i, graphY + (graphHeight - height), height, WHITE);  // Vẽ cột
    }

  } else {
    oled.setCursor(2, 14);
    oled.print("Chua ket noi WiFi!");
  }

  oled.display();
}

void displayDeviceInfo() {
  String deviceInfo[8];

  // Lấy thông tin phần cứng NGAY khi hàm được gọi
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  uint32_t flash_size;
  esp_flash_get_size(NULL, &flash_size);

  deviceInfo[0] = "Chip: ESP32";
  deviceInfo[1] = "Cores: " + String(chip_info.cores);
  deviceInfo[2] = "CPU Freq: " + String(getCpuFrequencyMhz()) + " MHz";
  deviceInfo[3] = "Flash: " + String(flash_size / (1024 * 1024)) + " MB";
  deviceInfo[4] = String(F("Firmware: ")) + F(FW_VERSION);
  deviceInfo[5] = "Free Heap: " + String(ESP.getFreeHeap() / 1024.0 / 1024.0, 2) + " MB";
  deviceInfo[6] = "Sketch: " + String(ESP.getSketchSize() / 1024.0 / 1024.0, 2) + " MB";
  deviceInfo[7] = "Rev: " + String(chip_info.revision);

  bool inInfoScreen = true;
  infoIndex = 0;

  while (inInfoScreen) {
    oled.fillRect(0, 13, 128, 52, BLACK);
    oled.setTextSize(1);
    oled.setTextColor(WHITE);

    for (int i = 0; i < SCREEN_LINES; i++) {
      int lineIndex = infoIndex + i;
      if (lineIndex >= 8) break;
      oled.setCursor(2, 14 + i * 12);
      oled.print(deviceInfo[lineIndex]);
    }

    oled.display();

    char key = keypad.getKey();
    if (key) {
      if (key == 'A' && infoIndex > 0) {
        infoIndex--;
      } else if (key == 'C' && infoIndex < 8 - SCREEN_LINES) {
        infoIndex++;
      } else if (key == 'D') {
        inInfoScreen = false;
      }
    }

    delay(50);
  }
}

void displayLogView() {
  oled.fillRect(0, 13, 128, 64, BLACK);
  oled.setCursor(0, 14);
  oled.println("Connect for view log");
  oled.setTextSize(2);
  oled.setCursor(0, 27);
  oled.println("esp.local");
  oled.fillRect(32, 0, 14, 11, BLACK);
  oled.display();
}
// Hiển thị Menu
void displayArrow() {
  oled.fillRect(0, 13, 20, 52, BLACK);
  drawBitmap(ArrowLeft, 2, 30, 16, 16);
  oled.fillRect(110, 13, 20, 52, BLACK);
  drawBitmap(ArrowRight, 111, 30, 16, 16);
}

void displayWifi() {
  oled.fillRect(20, 13, 90, 52, BLACK);
  drawBitmap(Wifi, 20, 13, 90, 52);
  oled.display();
};

void displayClock() {
  oled.fillRect(20, 13, 90, 52, BLACK);
  drawBitmap(Clock, 20, 13, 90, 52);
  oled.display();
};

void displaySetup() {
  oled.fillRect(20, 13, 90, 52, BLACK);
  drawBitmap(Setup, 20, 13, 90, 52);
  oled.display();
};

void displaySD() {
  oled.fillRect(20, 13, 90, 52, BLACK);
  drawBitmap(IconSD, 20, 13, 90, 52);
  oled.display();
};

void displayRF() {
  oled.fillRect(20, 13, 90, 52, BLACK);
  drawBitmap(IconRF, 20, 13, 90, 52);
  oled.display();
};

void displayJS() {
  oled.fillRect(20, 13, 90, 52, BLACK);
  drawBitmap(JS, 20, 13, 90, 52);
  oled.display();
};

// Hiển thị phần trăm pin
void displayBattery(int percent) {
  oled.fillRect(110, 3, 16, 5, BLACK);  // Xóa vùng hiển thị pin (16x5 px)

  // Vẽ các vạch pin theo phần trăm
  if (percent > 0) oled.fillRect(122, 3, 3, 5, WHITE);   // Vạch 1 (0-25%)
  if (percent > 25) oled.fillRect(118, 3, 3, 5, WHITE);  // Vạch 2 (26-50%)
  if (percent > 50) oled.fillRect(114, 3, 3, 5, WHITE);  // Vạch 3 (51-75%)
  if (percent > 75) oled.fillRect(110, 3, 3, 5, WHITE);  // Vạch 4 (76-100%)

  String percentText = String(percent) + "%";
  int textWidth = percentText.length() * 6;  // Tổng chiều rộng của chuỗi %

  // Vị trí phù hợp với vùng 21px cuối màn hình
  int xPos = 128 - 23 - textWidth;
  oled.fillRect(82, 1, 22, 9, BLACK);

  oled.setTextSize(1);
  oled.setCursor(xPos, 2);  // Hiển thị ngang hàng với biểu tượng pin
  oled.setTextColor(WHITE);
  oled.print(percentText);
  oled.display();
}

// Hàm cập nhật chữ
void updateDisplayText(int x, int y, const String& inputText) {
  oled.fillRect(x, y, 124, 8, BLACK);
  oled.fillRect(126, 38, 2, 14, WHITE);
  oled.setCursor(x, y);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.print(inputText);
  oled.display();
}

// Vẽ khung chứa chữ
void drawBorder(int x, int y) {
  oled.drawRect(x, y, 128, 14, WHITE);
  oled.drawRect(x + 1, y + 1, 126, 12, WHITE);
}

// Hiển thị giờ
const char* daysOfWeek[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
void displayTime() {
  if (WiFi.status() == WL_CONNECTED) {
    bool running = true;

    while (running) {
      updateTime();  // Cập nhật thời gian mới nhất

      oled.fillRect(0, 13, 128, 52, BLACK);
      oled.setTextColor(WHITE);

      // Hiển thị thứ (tiếng Anh) + ngày/tháng/năm
      oled.setTextSize(1);
      oled.setCursor(20, 25);
      oled.printf("%s, %02d/%02d/%04d",
                  daysOfWeek[p_tm->tm_wday],  // Lấy thứ bằng tiếng Anh
                  p_tm->tm_mday,
                  p_tm->tm_mon + 1,
                  p_tm->tm_year + 1900);

      // Hiển thị giờ:phút:giây
      oled.setTextSize(2);
      oled.setCursor(16, 35);
      oled.printf("%02d:%02d:%02d",
                  p_tm->tm_hour,
                  p_tm->tm_min,
                  p_tm->tm_sec);
      oled.display();

      // Kiểm tra phím thoát (D)
      char key = keypad.getKey();
      if (key == 'D') {
        running = false;  // Thoát vòng lặp khi nhấn D
      }

      delay(1000);  // Cập nhật thời gian mỗi giây
    }
  } else {
    oled.fillRect(0, 13, 128, 52, BLACK);
    oled.setCursor(2, 14);
    oled.print("Chua ket noi WiFi!");
    oled.display();
  }
}

// File trong thẻ SD
void showFileList() {
  readSDFiles(currentPath.c_str());

  if (fileList.empty()) {
    Serial.println("❌ Không có file nào trong thư mục.");
    oled.fillRect(0, 13, 128, 52, BLACK);
    oled.setCursor(0, 20);
    oled.print("No files found.");
    oled.display();
    delay(2000);
    return;
  }

  int selectedFile = 0;
  int fileMenuIndex = 0;

  while (true) {
    oled.fillRect(0, 13, 128, 52, BLACK);

    for (int i = 0; i < 3; i++) {
      int idx = fileMenuIndex + i;
      if (idx >= fileList.size()) break;

      oled.setCursor(0, 13 + i * 16);
      oled.setTextSize(1);
      oled.setTextColor(WHITE);

      if (idx == selectedFile) oled.print("> ");
      else oled.print("  ");

      if (fileList[idx].isDirectory)
        oled.print("/");  // folder có dấu /
      else
        oled.print(" ");

      oled.print(fileList[idx].name);
    }

    oled.display();

    char key = keypad.getKey();
    if (key) {
      if (key == 'A' && selectedFile > 0) {
        selectedFile--;
        if (selectedFile < fileMenuIndex) fileMenuIndex--;
      } else if (key == 'C' && selectedFile < fileList.size() - 1) {
        selectedFile++;
        if (selectedFile >= fileMenuIndex + 3) fileMenuIndex++;
      } else if (key == 'B') {
        if (fileList[selectedFile].isDirectory) {
          // Vào folder con
          currentPath = "/" + fileList[selectedFile].path;
          showFileList();  // gọi đệ quy để hiển thị folder mới
          return;
        } else {
          // Đã chọn file
          Serial.printf("📄 File được chọn: %s\n", fileList[selectedFile].path.c_str());
          // Xử lý file tại đây nếu cần
          return;
        }
      } else if (key == 'D') {
        // Quay lại folder cha
        if (currentPath != "/") {
          if (currentPath.endsWith("/")) currentPath.remove(currentPath.length() - 1);

          int lastSlash = currentPath.lastIndexOf('/');
          if (lastSlash >= 0) {
            currentPath = currentPath.substring(0, lastSlash);
            if (currentPath == "") currentPath = "/";
            showFileList();  // Hiển thị lại folder cha
            return;
          }
        } else {
          // Ở root thì thoát
          return;
        }
      }
    }

    delay(100);
  }
}

// RFID
void showWaitingRF() {
  oled.clearDisplay();
  displayHeader();
  displayWifiStatus();
  oled.setTextSize(1);
  oled.setCursor(0, 14);
  oled.println("Scan your card...");
  oled.display();
}

void showUID(String uid) {
  oled.clearDisplay();
  displayHeader();
  displayWifiStatus();
  oled.setTextSize(1);
  oled.setCursor(0, 14);
  oled.println("UID:");
  oled.setTextSize(2);
  oled.setCursor(0, 24);
  oled.println(uid);
  oled.display();
}
#endif