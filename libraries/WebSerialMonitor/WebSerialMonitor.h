#ifndef WEBSERIALMONITOR_H
#define WEBSERIALMONITOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
using WebSerialCommandCallback = std::function<void(const String&)>;
class WebSerialMonitor {
  public:
    WebSerialMonitor(AsyncWebServer* server);
    void begin();
    void println(int value);
    void println(const String& msg);
    void println(const char* msg);
    void printRaw(const char* msg);
    void cmdFromWeb(WebSerialCommandCallback cb);
  private:
    AsyncWebSocket ws;
    AsyncWebServer* _server;
    WebSerialCommandCallback _cmdCallback;
    void handleEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                     AwsEventType type, void* arg, uint8_t* data, size_t len);
};

#endif
