#include <Arduino.h>
#include <WiFi.h>

#include <ModbusTCPSlave.h>

#define SSID "ssid"
#define PASS "pass"

ModbusTCPSlave modbus(1, 502);

void onRequest(void* arg, const espModbus::Connection& connection) {
  switch (connection.request().functionalCode()) {
    case espModbus::READ_HOLD_REGISTER:
      {
      Serial.printf("New request %d: addr: %d - len %d", connection.request().functionalCode(), connection.request().address(), connection.request().noRegisters());
      size_t length = connection.request().noRegisters() * 2;  // registers to bytes
      uint8_t* data = new uint8_t[length];
      memset(data, 0x99, length);
      connection.respond(espModbus::SUCCES, data, length);
      delete[] data;
      return;
      }
    default:
      Serial.printf("Request not implemented");
      connection.respond(espModbus::ILLEGAL_FUNCTION);
      return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.print("Starting ESP\n");
  WiFi.persistent(false);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1);
  }

  Serial.print("WiFi connected\n");

  modbus.onRequest(onRequest);
  modbus.begin();
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    Serial.printf("free heap: %d\n", ESP.getFreeHeap());
  }
  delay(1);
}
