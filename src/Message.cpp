#include "Message.h"

namespace espModbus {

uint8_t low(uint16_t in) {
  return (in & 0xff);
}

uint8_t high(uint16_t in) {
  return ((in >> 8) & 0xff);
}

Message::Message(const Message& m) :
  _buffer(new uint8_t[m._length]),
  _length(m._length) {
    memcpy(_buffer, m._buffer, _length);
}

Message::Message(Message&& m) :
  _buffer(m._buffer),
  _length(m._length) {
    m._buffer = nullptr;
    m._length = 0;
}

Message::~Message() {
  if (_buffer)
    delete[] _buffer;
}

uint8_t* Message::data() const {
  return _buffer;
}

size_t Message::length() const {
  return _length;
}

uint16_t Message::transactionId() const {
  return (_buffer[0] << 8 | _buffer[1]);
}

uint8_t Message::slaveId() const {
  return _buffer[6];
}

FunctionalCode Message::functionalCode() const {
  return static_cast<FunctionalCode>(_buffer[7]);
}

uint16_t Message::address() const {
  return (_buffer[8] << 8 | _buffer[9]);
}
uint16_t Message::noRegisters() const {
  return (_buffer[10] << 8 | _buffer[11]);
}

Message::Message(uint16_t transactionId,
                 size_t length,
                 uint8_t slaveId) :
  _buffer(nullptr),
  _length(length + 7) {  // 7: length of MBAP header
    _buffer = new uint8_t[_length];
    memset(_buffer, 0, _length);
    _buffer[0] = high(transactionId);
    _buffer[1] = low(transactionId);
    _buffer[2] = 0;
    _buffer[3] = 0;
    _buffer[4] = high(length);
    _buffer[5] = low(length);
    _buffer[6] = slaveId;
  }


Request03::Request03(uint16_t transaction,
                     uint8_t slaveId,
                     uint16_t address,
                     uint16_t noRegisters) :
  RequestMessage(transaction, 5, slaveId) {
    _buffer[7] = READ_HOLD_REGISTER;
    _buffer[8] = high(address);
    _buffer[9] = low(address);
    _buffer[10] = high(noRegisters);
    _buffer[11] = low(noRegisters);
}

Message* Request03::createResponse(Error error, uint8_t* data, size_t len) const {
  Message* response = nullptr;
  if (error != SUCCES) {
    response = new ResponseError(transactionId(),
                                 slaveId(),
                                 functionalCode(),
                                 error);
  } else {
    response = new Response03(transactionId(),
                              slaveId(),
                              functionalCode(),
                              data,
                              len);
  }
  return response;
}

RequestMessage::RequestMessage(uint16_t transactionId,
                               size_t length,
                               uint8_t slaveId) :
  Message(transactionId, length, slaveId) {}

Response03::Response03(uint16_t transaction,
                       uint8_t slaveId,
                       uint16_t noRegisters,
                       uint8_t* data,
                       uint8_t len) :
  Message(transaction, (noRegisters * 2) + 2, slaveId) {
    _buffer[7] = READ_HOLD_REGISTER;
    size_t noBytes = noRegisters * 2;
    _buffer[8] = noBytes;
    for (size_t i = 0; i < len; ++i) {
      _buffer[9 + i] = data[i];
    }
  }

ResponseError::ResponseError(uint16_t transaction,
                             uint8_t slaveId,
                             FunctionalCode fc,
                             Error error) :
  Message(transaction, 2, slaveId) {
    _buffer[6] |= 0x80;
    _buffer[7] = error;
  }

} // end namespace espModbus
