#ifndef RFID_H
#define RFID_H

#include <SPI.h>
#include <MFRC522.h>
#include "deviceServer.h"
#include "Action.h"

#define SS_PIN 2
#define RST_PIN -1
MFRC522 RFID(SS_PIN, RST_PIN);

String readCard(unsigned long timeout = 0) {
  unsigned long startTime = millis();

  while (true) {
    if (RFID.PICC_IsNewCardPresent() && RFID.PICC_ReadCardSerial()) {
      String uid = "";
      for (byte i = 0; i < RFID.uid.size; i++) {
        if (RFID.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(RFID.uid.uidByte[i], HEX);
      }
      uid.toUpperCase();

      RFID.PICC_HaltA();
      RFID.PCD_StopCrypto1();
      return uid;
    }

    // Nếu có timeout và đã hết thời gian thì thoát
    if (timeout > 0 && millis() - startTime > timeout) {
      return "";
    }

    delay(10);  // Giảm tải CPU
  }
}

void startRFID() {
  SPI.begin();              // Bắt đầu giao tiếp SPI
  delay(100);               // Chờ ổn định kết nối SPI (rất quan trọng)

  RFID.PCD_Init();          // Khởi động module RC522
  delay(100);               // Chờ ổn định sau khi init

  // Kiểm tra kết nối RFID bằng cách đọc phiên bản firmware
  byte version = RFID.PCD_ReadRegister(RFID.VersionReg);
  if (version == 0x00 || version == 0xFF) {
    webSerial.println("❌ RFID not found");
    failSound(100);
  } else {
    webSerial.println("✅ RFID ready");
    verifySound(300);
  }
}

void endRFID() {
  RFID.PICC_HaltA();
  RFID.PCD_StopCrypto1();
  RFID.PCD_Reset();
  SPI.end();
  digitalWrite(SS_PIN, LOW);
  webSerial.println("✅ Removed RFID");
  verifySound(300);
}


#endif
