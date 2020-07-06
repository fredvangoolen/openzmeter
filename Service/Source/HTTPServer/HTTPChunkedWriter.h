// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <zlib.h>
#include <stdlib.h>
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace httplib;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
#define ZLIB_ENCODING_BLOCK   16384
// ---------------------------------------------------------------------------------------------------------------------
class HTTPChunkedWriter final {
  private :
    uint8_t   pOutBuff[ZLIB_ENCODING_BLOCK];
    z_stream  pStream;
    bool      pCompress;
    DataSink *pDataSink;
  public :
    HTTPChunkedWriter(Response &res, const char *mime, function<bool(HTTPChunkedWriter *writer)> callback);
    ~HTTPChunkedWriter();
    void SetSink(DataSink &sink);
    void Write(const string &data, bool flush = false);
};
// ---------------------------------------------------------------------------------------------------------------------