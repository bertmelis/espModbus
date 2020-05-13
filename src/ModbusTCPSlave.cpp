#include "ModbusTCPSlave.h"

ModbusTCPSlave::ModbusTCPSlave(uint8_t slaveId, uint16_t port) :
  _server(port),
  _slaveId(slaveId),
  _onRequestCb(nullptr),
  _arg(nullptr) {
}

ModbusTCPSlave::~ModbusTCPSlave() {
  // destructor of _server will call _server.end();
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

SemaphoreHandle_t ModbusTCPSlave::_semaphore = xSemaphoreCreateBinary();
uint8_t ModbusTCPSlave::_numberClients = 0;

void ModbusTCPSlave::_onClientConnect(void* slave, AsyncClient* client) {
  ModbusTCPSlave* s = static_cast<ModbusTCPSlave*>(slave);
  if (xSemaphoreTake(_semaphore, 500) == pdTRUE) {
    if (_numberClients < MAX_MODBUS_CLIENTS) {
      _numberClients++;
      espModbus::Connection* conn = new espModbus::Connection(s, client);
      if (conn == nullptr) {
        client->close(true);
        delete client;
      }
      xSemaphoreGive(_semaphore);
      return;
    }
  } else {
    log_e("couldn't obtain semaphore");
  }
  // semaphore not obtained or max clients reached
  client->close(true);
  delete client;
}

void ModbusTCPSlave::_onClientDisconnect(ModbusTCPSlave* c, espModbus::Connection* conn) {
  if (xSemaphoreTake(c->_semaphore, 500) == pdTRUE) {
    c->_numberClients--;
    delete conn;
    xSemaphoreGive(c->_semaphore);
  }
}

void ModbusTCPSlave::_onRequest(espModbus::Request* request) {
  _onRequestCb(_arg, request);
}