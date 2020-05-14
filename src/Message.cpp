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

#include "Message.h"

namespace espModbus {

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
    ++length;  // take byte for slaveId into account
    _buffer[4] = high(length);
    _buffer[5] = low(length);
    _buffer[6] = slaveId;
  }

RequestMessage::RequestMessage(uint16_t transactionId,
                               size_t length,
                               uint8_t slaveId) :
  Message(transactionId, length, slaveId) {}

Request01::Request01(uint16_t transaction,
                     uint8_t slaveId,
                     uint16_t address,
                     uint16_t noCoils) :
  RequestMessage(transaction, 5, slaveId) {
    _buffer[7] = READ_COILS;
    _buffer[8] = high(address);
    _buffer[9] = low(address);
    _buffer[10] = high(noCoils);
    _buffer[11] = low(noCoils);
}

ResponseMessage* Request01::createResponse(Error error, uint8_t* data, size_t len) const {
  ResponseMessage* response = nullptr;
  if (error == SUCCES) {
    response = new Response01(transactionId(),
                              slaveId(),
                              noRegisters(),
                              data,
                              len);
  } else {
    response = new ResponseError(transactionId(),
                                 slaveId(),
                                 functionalCode(),
                                 error);
  }
  return response;
}

Request02::Request02(uint16_t transaction,
                     uint8_t slaveId,
                     uint16_t address,
                     uint16_t noInputs) :
  RequestMessage(transaction, 5, slaveId) {
    _buffer[7] = READ_DISCR_INPUTS;
    _buffer[8] = high(address);
    _buffer[9] = low(address);
    _buffer[10] = high(noInputs);
    _buffer[11] = low(noInputs);
}

ResponseMessage* Request02::createResponse(Error error, uint8_t* data, size_t len) const {
  ResponseMessage* response = nullptr;
  if (error == SUCCES) {
    response = new Response02(transactionId(),
                              slaveId(),
                              noRegisters(),
                              data,
                              len);
  } else {
    response = new ResponseError(transactionId(),
                                 slaveId(),
                                 functionalCode(),
                                 error);
  }
  return response;
}

Request03::Request03(uint16_t transaction,
                     uint8_t slaveId,
                     uint16_t address,
                     uint16_t noRegisters) :
  RequestMessage(transaction, 5, slaveId) {
    _buffer[7] = READ_HOLD_REGISTERS;
    _buffer[8] = high(address);
    _buffer[9] = low(address);
    _buffer[10] = high(noRegisters);
    _buffer[11] = low(noRegisters);
}

ResponseMessage* Request03::createResponse(Error error, uint8_t* data, size_t len) const {
  ResponseMessage* response = nullptr;
  if (error == SUCCES) {
    response = new Response03(transactionId(),
                              slaveId(),
                              noRegisters(),
                              data,
                              len);
  } else {
    response = new ResponseError(transactionId(),
                                 slaveId(),
                                 functionalCode(),
                                 error);
  }
  return response;
}

ResponseMessage::ResponseMessage(uint16_t transactionId,
                                 size_t length,
                                 uint8_t slaveId) :
  Message(transactionId, length, slaveId) {}

Response01::Response01(uint16_t transaction,
                       uint8_t slaveId,
                       uint8_t noCoils,
                       uint8_t* data,
                       uint8_t len) :
  ResponseMessage(transaction, ((noCoils + 8 - 1) / 8) + 2, slaveId) {
    _buffer[7] = READ_COILS;
    size_t noBytes = (noCoils + 8 - 1) / 8;
    _buffer[8] = noBytes;
    for (size_t i = 0; i < len; ++i) {
      _buffer[9 + i] = data[i];
    }
  }

Response02::Response02(uint16_t transaction,
                       uint8_t slaveId,
                       uint8_t noInputs,
                       uint8_t* data,
                       uint8_t len) :
  ResponseMessage(transaction, ((noInputs + 8 - 1) / 8) + 2, slaveId) {
    _buffer[7] = READ_DISCR_INPUTS;
    size_t noBytes = (noInputs + 8 - 1) / 8;
    _buffer[8] = noBytes;
    for (size_t i = 0; i < len; ++i) {
      _buffer[9 + i] = data[i];
    }
  }

Response03::Response03(uint16_t transaction,
                       uint8_t slaveId,
                       uint8_t noRegisters,
                       uint8_t* data,
                       uint8_t len) :
  ResponseMessage(transaction, (noRegisters * 2) + 2, slaveId) {
    _buffer[7] = READ_HOLD_REGISTERS;
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
  ResponseMessage(transaction, 2, slaveId) {
    _buffer[6] |= 0x80;
    _buffer[7] = error;
  }

}  // end namespace espModbus
