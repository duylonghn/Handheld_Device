#ifndef READ_SD_H
#define READ_SD_H

#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <vector>
#include "deviceServer.h"
#include "Action.h"

#define SD_CS 5

std::vector<String> fileList;  // Danh sách file

// 🗂 Đọc danh sách file từ thư mục "/Data/"
void listFilesInFolder(const char* folderPath) {
  fileList.clear();  // Xóa danh sách cũ
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    webSerial.println("❌ Không thể khởi động SD Card!");
    failSound(100);
    return;
  }
  webSerial.println("✅ SD Card đã sẵn sàng");
  verifySound(200);

  File root = SD.open(folderPath);
  if (!root) {
    webSerial.println("❌ Không tìm thấy thư mục!");
    return;
  }

  webSerial.println("📂 Danh sách file trong thư mục:");
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      fileList.push_back(file.name());  // Lưu tên file vào danh sách
    }
    file = root.openNextFile();
  }

  webSerial.println("✅ Hoàn thành!");
}

#endif
