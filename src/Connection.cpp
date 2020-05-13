#include "ModbusTCPSlave.h"

namespace espModbus {

Connection::Connection(ModbusTCPSlave* slave, AsyncClient* client) :
  _server(slave),
  _client(client),
  _factory(),
  _keepaliveCount(0),
  _sndQueue(MAX_MODBUS_REQUESTS * 2) {
    _client->onPoll(_onPoll, this);
    _client->onData(_onData, this);
    _client->onDisconnect(_onDisconnect, this);
  }

Connection::~Connection() {
  while (_sndQueue.size() > 0) {
    delete _sndQueue.front();
    _sndQueue.pop();
  }
}

bool Connection::send(Message* message) {
  log_v("msg sent to outbox");
  if (_sndQueue.size() < MAX_MODBUS_REQUESTS * 2) {
    _sndQueue.push(std::move(message));
    return true;
  } else {
    log_e("outbox full");
    return false;
  }
}

void Connection::_onData(void* conn, AsyncClient* client, void* data, size_t len) {
  log_v("data rx - len: %d", len);
  Connection* c = static_cast<Connection*>(conn);
  c->_keepaliveCount = 0;
  RequestMessage* message = nullptr;
  while (len) {
    len -= c->_factory.parse(static_cast<uint8_t*>(data), len, message);
    if (message) {
      Request* request = new Request(c, std::move(message));
      c->_server->_onRequest(request);
      delete request;
      message = nullptr;
    }
  }
}

void Connection::_onPoll(void* conn, AsyncClient* client) {
  Connection* c = static_cast<Connection*>(conn);
  // polling is about every 500ms so timeout in 60 min = after 120 counts
  ++(c->_keepaliveCount);
  if (c->_keepaliveCount > 120) {
    c->_client->close(false);
  }

  // check outbox
  if (c->_sndQueue.size() > 0) {
    if (c->_client->space() >= (c->_sndQueue.front()->length())) {
      c->_client->write(reinterpret_cast<const char*>(c->_sndQueue.front()->data()),
                        c->_sndQueue.front()->length());
      c->_keepaliveCount = 0;
      delete c->_sndQueue.front();
      c->_sndQueue.pop();
    }
  }
}

void Connection::_onDisconnect(void* conn, AsyncClient* client) {
  log_v("client disconnected");
  delete client;
  Connection* c = static_cast<Connection*>(conn);
  c->_server->_onClientDisconnect(c->_server, c);
}

} // end namespace espModbus
