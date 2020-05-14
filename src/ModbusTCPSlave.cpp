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

#include "ModbusTCPSlave.h"

uint8_t ModbusTCPSlave::_numberClients = 0;

ModbusTCPSlave::ModbusTCPSlave(uint8_t slaveId, uint16_t port) :
  _server(port),
  _slaveId(slaveId),
  _semaphore(nullptr),
  _onRequestCb(nullptr),
  _arg(nullptr) {
    _semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(_semaphore);
}

ModbusTCPSlave::~ModbusTCPSlave() {
  // destructor of _server will call _server.end();
  // TODO(bertmelis): what about current clients?
}

void ModbusTCPSlave::onRequest(espModbus::OnRequestCb callback, void* arg) {
  _onRequestCb = callback;
  _arg = arg;
}

void ModbusTCPSlave::begin() {
  if (!_onRequestCb) {
    log_e("onRequest callback mandatory, aborting");
    abort();
  }
  _server.setNoDelay(true);
  _server.onClient(_onClientConnect, this);
  _server.begin();
}

uint8_t ModbusTCPSlave::getId() const {
  return _slaveId;
}

void ModbusTCPSlave::_onClientConnect(void* slave, AsyncClient* client) {
  ModbusTCPSlave* s = static_cast<ModbusTCPSlave*>(slave);
  if (xSemaphoreTake(s->_semaphore, 500) == pdTRUE) {
    if (_numberClients < MAX_MODBUS_CLIENTS) {
      _numberClients++;
      espModbus::Connection* conn = new espModbus::Connection(s, client);
      if (conn != nullptr) {
        xSemaphoreGive(s->_semaphore);
        return;
      }
    }
    xSemaphoreGive(s->_semaphore);
    client->close(true);
    delete client;
  } else {
    log_e("couldn't obtain semaphore");
    client->close(true);
    delete client;
  }
}

void ModbusTCPSlave::_onClientDisconnect(ModbusTCPSlave* c, espModbus::Connection* conn) {
  if (xSemaphoreTake(c->_semaphore, 500) == pdTRUE) {
    c->_numberClients--;
    delete conn;
    xSemaphoreGive(c->_semaphore);
  }
}

void ModbusTCPSlave::_onRequest(const espModbus::Connection& connection) {
  _onRequestCb(_arg, connection);
}
