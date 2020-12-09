/* espModbusSlave

Copyright 2020 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

// unit: times 500ms
#ifndef CLIENT_KEEPALIVE
#define CLIENT_KEEPALIVE 120
#endif

#ifndef MAX_MODBUS_CLIENTS
#define MAX_MODBUS_CLIENTS 3
#endif

#ifndef MAX_MODBUS_REQUESTS
#define MAX_MODBUS_REQUESTS 5
#endif

// general purpose
#include <functional>  // std::function
#include <utility>  // std::move

// framework
#include <FreeRTOS.h>  // must appear before smphr.h
#include <freertos/semphr.h>
#include <esp32-hal.h>  // logging and millis()

// external
#include <AsyncTCP.h>

// internal
#include "Helpers.h"
#include "MessageParser.h"
#include "Message.h"

namespace espModbus {
class Request;
class Connection;
typedef std::function<void(void*, const espModbus::Connection&)> OnRequestCb;
}
class ModbusTCPSlave;

namespace espModbus {

class Connection {
 public:
  Connection(ModbusTCPSlave* slave, AsyncClient* client);
  ~Connection();
  const Message& request() const;
  bool respond(Error error, uint8_t* data = nullptr, size_t len = 0) const;

 private:
  static void _onData(void* conn, AsyncClient* client, void* data, size_t len);
  static void _onPoll(void* conn, AsyncClient* client);
  static void _onDisconnect(void* conn, AsyncClient* client);

  ModbusTCPSlave* _slave;
  AsyncClient* _client;
  MessageParser<RequestMessage*> _factory;
  RequestMessage* _currentRequest;
  uint8_t _keepaliveCount;
};

}  // end namespace espModbus

class ModbusTCPSlave {
  friend class espModbus::Connection;

 public:
  explicit ModbusTCPSlave(uint8_t slaveId, uint16_t port = 502);
  ~ModbusTCPSlave();
  void onRequest(espModbus::OnRequestCb callback, void* arg = nullptr);
  void begin();
  uint8_t getId() const;

 private:
  static void _onClientConnect(void* arg, AsyncClient* client);
  static void _onClientDisconnect(ModbusTCPSlave* c, espModbus::Connection* conn);
  void _onRequest(const espModbus::Connection& connection);

  AsyncServer _server;
  uint8_t _slaveId;
  SemaphoreHandle_t _semaphore;
  static uint8_t _numberClients;
  espModbus::OnRequestCb _onRequestCb;
  void* _arg;
};
