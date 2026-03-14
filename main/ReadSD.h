#ifndef READ_SD_H
#define READ_SD_H

#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <vector>
#include "deviceServer.h"
#include "Action.h"

#define SD_CS 5

struct FileEntry {
  String name;
  String path;
  bool isDirectory;
};

std::vector<FileEntry> fileList;
String currentPath = "/";

void readSDFiles(const char* folderPath) {
  fileList.clear();
  currentPath = String(folderPath);  // Cập nhật đường dẫn hiện tại

  if (!SD.begin(SD_CS)) {
    webSerial.println("❌ Không thể khởi động SD Card!");
    failSound(100);
    return;
  }

  delay(100);  // Cho SD ổn định (nên có nếu gắn/rút thẻ)

  File root = SD.open(folderPath);
  if (!root || !root.isDirectory()) {
    webSerial.println("❌ Không thể mở thư mục: " + String(folderPath));
    return;
  }

  File file = root.openNextFile();
  while (file) {
    FileEntry entry;

    String fullName = String(file.name());
    int lastSlash = fullName.lastIndexOf('/');
    if (lastSlash != -1)
      entry.name = fullName.substring(lastSlash + 1);
    else
      entry.name = fullName;

    entry.path = String(file.name());  // Path đầy đủ để truy cập lại
    entry.isDirectory = file.isDirectory();

    fileList.push_back(entry);
    file = root.openNextFile();
  }
}

#endif
