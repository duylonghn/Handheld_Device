#include "WebSerialMonitor.h"
#include "SerialMonitor.h"

WebSerialMonitor::WebSerialMonitor(AsyncWebServer* server)
  : ws("/"), _server(server) {}

void WebSerialMonitor::begin() {

  /*_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", webserial);
  });//*/

  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", webserial, sizeof(webserial));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  ws.onEvent([this](AsyncWebSocket* server,
                    AsyncWebSocketClient* client,
                    AwsEventType type,
                    void* arg,
                    uint8_t* data,
                    size_t len) {
    handleEvent(server, client, type, arg, data, len);
  });

  _server->addHandler(&ws);
  Serial.println("✅ Truy cập: http://" + WiFi.localIP().toString() + " để vào WebSerialMonitor");
}

void WebSerialMonitor::cmdFromWeb(WebSerialCommandCallback cb) {
  _cmdCallback = cb;
}

void WebSerialMonitor::println(int value) {
  ws.textAll(String(value));
} 

void WebSerialMonitor::println(const String& msg) {
  ws.textAll(msg + "\n"); // Thêm ký tự xuống dòng
}

void WebSerialMonitor::println(const char* msg) {
  ws.textAll(String(msg) + "\n"); // Chuyển sang String và thêm \n
}

void WebSerialMonitor::printRaw(const char* msg) {
  ws.textAll(msg); // Không thay đổi, gửi trực tiếp
}

void WebSerialMonitor::handleEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                   AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("📡 Client #%u đã kết nối\n", client->id());
    client->text("✅ ESP32 đã kết nối WebSerial");
  }
  else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("🔌 Client #%u ngắt kết nối\n", client->id());
  }
  else if (type == WS_EVT_DATA) {
    String received = "";
    for (size_t i = 0; i < len; i++) received += (char)data[i];
    received.trim();

    Serial.printf("%s\n", received.c_str());

    if (_cmdCallback) _cmdCallback(received);

    client->text("📥 Received: " + received);
  }
}
