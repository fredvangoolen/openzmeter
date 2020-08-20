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
#include <algorithm>
#include "URL.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
URL::URL(const string &url) {
  typedef string::const_iterator iterator_t;
  if(url.length() == 0) return;
  iterator_t uriEnd = url.end();
  iterator_t queryStart = find(url.begin(), uriEnd, '?');
  iterator_t protocolStart = url.begin();
  iterator_t protocolEnd = find(protocolStart, uriEnd, ':');
  pPort = 80;
  if(protocolEnd != uriEnd) {
    std::string prot = &*(protocolEnd);
    if((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
      pProtocol = string(protocolStart, protocolEnd);
      protocolEnd += 3;
      if(pProtocol == "https") pPort = 443;
      if(pProtocol == "mqtt") pPort = 1883;
    } else {
      protocolEnd = url.begin();
    }
  } else {
    protocolEnd = url.begin();
  }
  iterator_t hostStart = protocolEnd;
  iterator_t pathStart = find(hostStart, uriEnd, '/');
  iterator_t hostEnd = find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart, ':');
  pHost = string(hostStart, hostEnd);
  if((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) {
    hostEnd++;
    iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
    string PortText = string(hostEnd, portEnd);
    pPort = (PortText == "") ? 80 : atoi(PortText.c_str());
  }
  if(pathStart != uriEnd) pPath = string(pathStart, queryStart);
  if(queryStart != uriEnd) pQuery = string(queryStart, url.end());
}
URL::Protocol_t URL::Protocol() const {
  return Protocol_t::UNKNOWN;
}
uint16_t URL::Port() const {
  return pPort;
}
string URL::Host() const {
  return pHost;
}
string URL::Path() const {
  return pPath;
}
string URL::Query() const {
  return pQuery;
}
//----------------------------------------------------------------------------------------------------------------------