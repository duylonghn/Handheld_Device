#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

const int xPin = 39;
const int yPin = 36;
const int SwPin = 15;

// Cấu trúc trả về dữ liệu joystick
struct JoystickData {
  int x;
  int y;
  int button;
};

// Hàm đọc dữ liệu từ joystick
JoystickData readJoystick() {
  JoystickData data;
  data.x = analogRead(xPin);
  data.y = analogRead(yPin);
  data.button = digitalRead(SwPin);
  return data;
}

// Hàm khởi tạo (gọi trong setup)
void initJoystick() {
  pinMode(SwPin, INPUT_PULLUP);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
}

#endif
