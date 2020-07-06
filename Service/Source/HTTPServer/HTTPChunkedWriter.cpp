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
#include "HTTPChunkedWriter.h"
// ---------------------------------------------------------------------------------------------------------------------
HTTPChunkedWriter::HTTPChunkedWriter(Response &res, const char *mime, function<bool(HTTPChunkedWriter *writer)> callback) {
  res.set_header("Content-Type", mime);
  res.status = HTTP_OK;
  pDataSink = NULL;
  if(res.accept_gzip_encoding == true) {
    pStream.zalloc    = Z_NULL;
    pStream.zfree     = Z_NULL;
    pStream.opaque    = Z_NULL;
    if(deflateInit2(&pStream, res.gzip_compression_level, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) == Z_OK) {
      pCompress = true;
      res.set_header("Content-Encoding", "gzip");
    }
  }
  res.set_chunked_content_provider(
    [=](size_t offset, DataSink &sink) -> bool {
      SetSink(sink);
      while(callback(this) == true);
      return false;
    },
    [=]() { delete this; }
  );
}
HTTPChunkedWriter::~HTTPChunkedWriter() {
  if(pCompress == true) deflateEnd(&pStream);
}
void HTTPChunkedWriter::SetSink(DataSink &sink) {
  pDataSink = &sink;
}
void HTTPChunkedWriter::Write(const string &data, bool flush) {
  if(pDataSink == NULL) return;
  if(pCompress == false) {
    pDataSink->write(data.c_str(), data.size());
  } else {
    pStream.avail_in = data.size();
    pStream.next_in  = (uint8_t*)data.c_str();
    do {
      pStream.avail_out = ZLIB_ENCODING_BLOCK;
      pStream.next_out  = pOutBuff;
      deflate(&pStream, flush ? Z_FINISH : Z_NO_FLUSH);
      size_t Count = (ZLIB_ENCODING_BLOCK - pStream.avail_out);
      if(Count > 0) pDataSink->write((char*)pOutBuff, Count);
    } while(pStream.avail_out == 0);
  }
  if(flush) pDataSink->done();
}
// ---------------------------------------------------------------------------------------------------------------------