// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/status.h"

#include <cstdio>

#include "port/port.h"

/*:  This file contains implementation of
 *  - CopyState
 *  - Status(x, y, z)
 *  - ToString
 */

namespace leveldb {

const char* Status::CopyState(const char* state) {
  uint32_t size;
  //: fetch the first 4 bytes in state, it's the length of the msg
  std::memcpy(&size, state, sizeof(size));
  //: create a new state (4 for size, 1 for code, and size for message)
  char* result = new char[size + 5];
  //: copy to new state
  std::memcpy(result, state, size + 5);
  return result;
}

Status::Status(Code code, const Slice& msg, const Slice& msg2) {
  assert(code != kOk);
  const uint32_t len1 = static_cast<uint32_t>(msg.size());
  const uint32_t len2 = static_cast<uint32_t>(msg2.size());
  const uint32_t size = len1 + (len2 ? (2 + len2) : 0);  //: 2 for ': '
  char* result = new char[size + 5]; //: first 4 bytes for msg length and 1 byte for code
  //: copy size
  std::memcpy(result, &size, sizeof(size));
  //: copy code
  result[4] = static_cast<char>(code);
  //: copy message
  std::memcpy(result + 5, msg.data(), len1);
  if (len2) {
    result[5 + len1] = ':';
    result[6 + len1] = ' ';
    std::memcpy(result + 7 + len1, msg2.data(), len2);
  }
  state_ = result;
}

std::string Status::ToString() const {
  if (state_ == nullptr) {
    return "OK";
  } else {
    char tmp[30];
    const char* type;
    switch (code()) {
      case kOk:
        type = "OK";
        break;
      case kNotFound:
        type = "NotFound: ";
        break;
      case kCorruption:
        type = "Corruption: ";
        break;
      case kNotSupported:
        type = "Not implemented: ";
        break;
      case kInvalidArgument:
        type = "Invalid argument: ";
        break;
      case kIOError:
        type = "IO error: ";
        break;
      default:
        std::snprintf(tmp, sizeof(tmp),
                      "Unknown code(%d): ", static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    //: concat type and message to form the final string
    uint32_t length;
    std::memcpy(&length, state_, sizeof(length)); //: fetch the first 4 bytes for msg length
    result.append(state_ + 5, length);  //: skip first 5 bytes to read the msg; concat type and msg
    return result;
  }
}

}  // namespace leveldb
