#include "ModbusTCPSlave.h"

namespace espModbus {

Request::Request(Connection* c, RequestMessage* m) :
  message(m),
  _conn(c) {}

void Request::respond(Error error, uint8_t* data, size_t len) {
  Message* r = message->createResponse(error, data, len);
  _conn->send(r);
  delete r;
}

}  // end namespace espModbus