#ifndef SETTING_TIME_H
#define SETTING_TIME_H

#include <time.h>

#define TIMEZONE_OFFSET 7 * 3600  // Múi giờ GMT+7
#define DST_OFFSET 0  // Không sử dụng giờ mùa hè

bool connectNTP() {
  const unsigned long timeout = 10000; // 10 giây timeout
  unsigned long startTime = millis();  // Lấy thời gian bắt đầu

  Serial.println("Connecting to NTP server...");

  // Cấu hình NTP
  configTime(TIMEZONE_OFFSET, DST_OFFSET, "pool.ntp.org", "time.nist.gov");

  while (!time(nullptr)) {
    if (millis() - startTime >= timeout) {
      Serial.println("NTP connection timeout!");
      return false;  // Quá thời gian kết nối
    }
    Serial.print(".");  // Hiển thị trạng thái
    delay(1000);
  }
  
  Serial.println("\nNTP time synchronized.");
  return true;  // Kết nối thành công
}

// Khai báo biến thời gian
time_t now;
struct tm* p_tm;

void updateTime() {
  // Lấy thời gian hiện tại và cập nhật biến toàn cục
  now = time(nullptr);
  p_tm = localtime(&now);
};

void displayTimeSerial() {
  updateTime();
  
  // Hiển thị qua Serial
  Serial.print(p_tm->tm_mday);
  Serial.print("/");
  Serial.print(p_tm->tm_mon + 1);
  Serial.print("/");
  Serial.print(p_tm->tm_year + 1900);
  
  Serial.print(" ");
  
  Serial.print(p_tm->tm_hour);
  Serial.print(":");
  Serial.print(p_tm->tm_min);
  Serial.print(":");
  Serial.println(p_tm->tm_sec);
};

#endif