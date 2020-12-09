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

namespace espModbus {

inline uint8_t low(uint16_t in) {
  return (in & 0xff);
}

inline uint8_t high(uint16_t in) {
  return ((in >> 8) & 0xff);
}

inline uint8_t coilsToBytes(uint8_t noCoils) {
  return (noCoils + 8 - 1) / 8;
}

inline uint8_t inputsToBytes(uint8_t noInputs) {
  return coilsToBytes(noInputs);
}

inline uint8_t registersToBytes(uint8_t noRegisters) {
  return noRegisters * 2;
}

}  // end namespace espModbus