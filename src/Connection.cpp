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

namespace espModbus {

Connection::Connection(ModbusTCPSlave* slave, AsyncClient* client) :
  _slave(slave),
  _client(client),
  _factory(),
  _currentRequest(nullptr),
  _keepaliveCount(0) {
    _client->onPoll(_onPoll, this);
    _client->onData(_onData, this);
    _client->onDisconnect(_onDisconnect, this);
  }

Connection::~Connection() {
  delete _client;
}

const Message& Connection::request() const {
  return *_currentRequest;
}

bool Connection::respond(Error error, uint8_t* data, size_t len) const {
  bool result = false;
  ResponseMessage* response = _currentRequest->createResponse(error, data, len);
  log_v("sending message, len %d", response->length());
  if (_client->space() > response->length()) {
    _client->write(reinterpret_cast<const char*>(response->data()), response->length());
    log_v("sent!");
    result = true;
  } else {
    log_e("unable to send");
  }
  delete response;
  return result;
}

void Connection::_onData(void* conn, AsyncClient* client, void* data, size_t len) {
  log_v("data rx - len: %d", len);
  Connection* c = static_cast<Connection*>(conn);
  c->_keepaliveCount = 0;
  size_t parsed = 0;
  while (len > 0) {
    parsed = c->_factory.parse(static_cast<uint8_t*>(data), len, c->_currentRequest);
    len = len - parsed;
    log_v("parsed: %d", parsed);
    if (c->_currentRequest != nullptr) {
      c->_slave->_onRequest(*c);
      delete c->_currentRequest;
      c->_currentRequest = nullptr;
    }
  }
}

void Connection::_onPoll(void* conn, AsyncClient* client) {
  Connection* c = static_cast<Connection*>(conn);
  // polling is about every 500ms so timeout in 60 min = after 120 counts
  ++(c->_keepaliveCount);
  if (c->_keepaliveCount > 120) {
    log_v("client %d inactive, closing", client);
    c->_client->close(false);
  }
}

void Connection::_onDisconnect(void* conn, AsyncClient* client) {
  log_v("client disconnected");
  Connection* c = static_cast<Connection*>(conn);
  c->_slave->_onClientDisconnect(c->_slave, c);
}

}  // end namespace espModbus
