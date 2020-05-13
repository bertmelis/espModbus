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

#include <stdint.h>  // for uint*_t
#include <stddef.h>  // for size_t
#include <cstring>  // for memcpy

#include "TypeDefs.h"

namespace espModbus {

class MessageFactory;

class Message {
 public:
  Message(const Message& m);
  Message(Message&& m);
  virtual ~Message();

  uint8_t* data() const;
  size_t length() const;
  uint16_t transactionId() const;
  uint8_t slaveId() const;
  FunctionalCode functionalCode() const;
  uint16_t address() const;
  uint16_t noRegisters() const;

 protected:
  Message(uint16_t transactionId,
          size_t length,
          uint8_t slaveId);
  uint8_t* _buffer;
  size_t _length;
};

class RequestMessage : public Message {
 public:
  virtual Message* createResponse(Error error, uint8_t* data = nullptr, size_t len = 0) const = 0;

 protected:
  RequestMessage(uint16_t transactionId,
                 size_t length,
                uint8_t slaveId);
};

class Request03 : public RequestMessage {
 public:
  Request03(uint16_t transaction,
            uint8_t slaveId,
            uint16_t address,
            uint16_t noRegisters);
  virtual Message* createResponse(Error error, uint8_t* data = nullptr, size_t len = 0) const;
};

class Response03 : public Message {
 public:
  Response03(uint16_t transaction,
             uint8_t slaveId,
             uint16_t noRegisters,
             uint8_t* data,
             uint8_t len);
};

class ResponseError : public Message {
 public:
  ResponseError(uint16_t transaction,
                uint8_t slaveId,
                FunctionalCode fc,
                Error error);
};

}  // end namespace espModbus