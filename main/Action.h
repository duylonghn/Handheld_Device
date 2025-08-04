#ifndef ACTION_H
#define ACTION_H

#define BUZZ 17
#define PIN_BATTERY 34      // Dùng GPIO34 để đọc điện áp pin
#define MAX_BATTERY 4.2     // Điện áp pin đầy
#define MIN_BATTERY 3.0     // Điện áp pin cạn

float readBatteryVoltage() {
  int raw = analogRead(PIN_BATTERY);
  float voltage = raw * (3.3 / 4095.0) * ((100.0 + 47.0) / 47.0); // Hệ số phân áp
  return voltage;
}

int batteryPercentage() {
  float voltage = readBatteryVoltage();
  int percent = map(voltage * 100, MIN_BATTERY * 100, MAX_BATTERY * 100, 0, 100);
  return constrain(percent, 0, 100); // Giới hạn từ 0 - 100%
  delay(2000);
}

void verifySound(int time) {
  digitalWrite(BUZZ, HIGH);
  delay(time);
  digitalWrite(BUZZ, LOW);
}
void failSound(int time) {
  digitalWrite(BUZZ, HIGH);
  delay(time);
  digitalWrite(BUZZ, LOW);
  delay(time);
  digitalWrite(BUZZ, HIGH);
  delay(time);
  digitalWrite(BUZZ, LOW);
}
#endif
