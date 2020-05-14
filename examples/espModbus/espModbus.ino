#include <Arduino.h>
#include <WiFi.h>

#include <ModbusTCPSlave.h>

#define SSID "ssid"
#define PASS "pass"

ModbusTCPSlave modbus(1, 502);

void onRequest(void* arg, const espModbus::Connection& connection) {
  Serial.print("New request: ");
  for (size_t i = 0; i < connection.request().length(); ++i) {
    Serial.printf("%02x ", connection.request().data()[i]);
  }
  Serial.print("\n");
  switch (connection.request().functionalCode()) {
    case espModbus::READ_COILS:
      {
      size_t noBytes = espModbus::coilsToBytes(connection.request().noRegisters());
      uint8_t* data = new uint8_t[noBytes];
      memset(data, 0x10, noBytes);  // <-- fill in actual data
      connection.respond(espModbus::SUCCES, data, noBytes);
      delete[] data;
      return;
      }
    case espModbus::READ_DISCR_INPUTS:
      {
      size_t noBytes = espModbus::inputsToBytes(connection.request().noRegisters());
      uint8_t* data = new uint8_t[noBytes];
      memset(data, 0x20, noBytes);  // <-- fill in actual data
      connection.respond(espModbus::SUCCES, data, noBytes);
      delete[] data;
      return;
      }
    case espModbus::READ_HOLD_REGISTERS:
      {
      size_t noBytes = espModbus::registersToBytes(connection.request().noRegisters());
      uint8_t* data = new uint8_t[noBytes];
      memset(data, 0x30, noBytes);  // <-- fill in actual data
      connection.respond(espModbus::SUCCES, data, noBytes);
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
