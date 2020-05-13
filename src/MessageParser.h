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

#include <algorithm>  // std::min

#include <esp32-hal-log.h>

#include "Message.h"
#include "SimpleQueue.h"

#ifndef PARSER_BUFFER_LENGTH
#define PARSER_BUFFER_LENGTH 265  // 256 data + 7 MBAP + 1 FC + 1 LEN
#endif


namespace espModbus {

template <class T>  // must support .create(...);
class MessageParser {
 public:
  MessageParser() :
    _buffer(PARSER_BUFFER_LENGTH) {}

  size_t parse(uint8_t* data, size_t len, T* message) {
    // shortest message is 12 bytes, so try to copy in chuncks of 12 bytes
    size_t length = std::min(PARSER_BUFFER_LENGTH - _buffer.size(), len);
    log_v("%d copied", length);
    for (size_t i = 0; i < length; ++i) {
      _buffer.push(data[i]);
    }

    // start validating
    while (_buffer.size() >= 12) {
      if (_buffer[2] != 0 ||  // high byte protocol
          _buffer[3] != 0 ||  // low byte protocol
          _buffer[4] != 0) {  // high byte length == 0, length is max 256
        log_w("protocol error");
        _buffer.pop();
        continue;
      }
      if (_buffer[7] == READ_HOLD_REGISTER &&
          _buffer[5] == 6 &&
          _buffer[11] < 123) {
        log_v("modbus message valid");
        message = new Request03((_buffer[0] << 8 | _buffer[1]),
                                _buffer[6],
                                (_buffer[8] << 8 | _buffer[9]),
                                _buffer[11]);
        _buffer.pop(12);
        break;
      } else {
        _buffer.pop();
        continue;
      }
    }
    return length;
  }

 private:
  SimpleQueue<uint8_t> _buffer;
};

}  // end namespace espModbus
