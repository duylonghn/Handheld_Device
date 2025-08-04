#ifndef SETTING_KEYPAD_H
#define SETTING_KEYPAD_H

#include "GUI.h"
#include <Keypad.h>

// Cấu hình bàn phím ma trận 4x4
const byte ROWS = 4;
const byte COLS = 4;

// Bố trí nút bấm trên bàn phím ma trận
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }  // Phím * sẽ dùng để xóa
};

// Chân kết nối hàng (ROW) trên ESP32
byte rowPins[ROWS] = { 14, 27, 26, 25 };

// Chân kết nối cột (COL) trên ESP32
byte colPins[COLS] = { 4, 13, 33, 32 };

// Tạo đối tượng keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// T9 Mapping cho các phím số
const char t9Mapping[10][6] = {
  { '0', ' ' },                      // Phím 0: Nhấn nhập số 0, giữ nhập khoảng trắng
  { '1', '.', ',', '@', '#', '!' },  // Phím 1: Ký tự đặc biệt
  { '2', 'A', 'B', 'C' },            // Phím 2
  { '3', 'D', 'E', 'F' },            // Phím 3
  { '4', 'G', 'H', 'I' },            // Phím 4
  { '5', 'J', 'K', 'L' },            // Phím 5
  { '6', 'M', 'N', 'O' },            // Phím 6
  { '7', 'P', 'Q', 'R', 'S' },       // Phím 7
  { '8', 'T', 'U', 'V' },            // Phím 8
  { '9', 'W', 'X', 'Y', 'Z' }        // Phím 9
};

// Biến lưu trữ trạng thái nhập liệu
String inputText = "";
bool uppercase = false;
unsigned long lastPressTime = 0;
char lastKey = '\0';
int keyIndex = 0;

String getTextInput(int x, int y) {
  inputText = "";  // Reset biến toàn cục
  lastKey = '\0';  // Đặt lại phím cuối cùng
  keyIndex = 0;    // Đặt lại chỉ mục T9

  while (true) {
    char key = keypad.getKey();
    unsigned long currentTime = millis();

    if (key) {
      if (isdigit(key)) {  // Nếu là số
        if (key == lastKey && (currentTime - lastPressTime) < 500) {
          // Nếu bấm cùng phím trong 500ms, chuyển sang chữ tiếp theo
          int charCount = strlen(t9Mapping[key - '0']);
          keyIndex = (keyIndex + 1) % charCount;
          inputText[inputText.length() - 1] = uppercase ? t9Mapping[key - '0'][keyIndex]
                                                        : tolower(t9Mapping[key - '0'][keyIndex]);
        } else {
          // Nếu bấm phím mới hoặc sau 500ms, thêm số vào chuỗi
          inputText += key;
          keyIndex = 0;
        }
      } else if (key == '*') {  // Xóa ký tự cuối
        if (!inputText.isEmpty()) {
          inputText.remove(inputText.length() - 1);
        }
      } else if (key == '#') {  // Chuyển đổi chữ hoa/thường
        uppercase = !uppercase;
      } else if (key == 'B') {  // Nhấn 'B' để hoàn tất nhập
        return inputText;
      } else if (key == 'D') {  
        return "EXIT";  // Nếu nhấn D, trả về "EXIT" để thoát
      }

      // Cập nhật hiển thị
      updateDisplayText(x, y, inputText);

      // Cập nhật trạng thái phím
      lastKey = key;
      lastPressTime = currentTime;
    }
  }
}


#endif
