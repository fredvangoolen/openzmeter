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
#include "HTTPRequest.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
HTTPRequest::HTTPRequest(const Request &request, Response *response) {
  pResponse = response;
  Error("", HTTP_NOT_FOUND);
  if(request.has_header("Accept") == false) {
    Error("'Accept' header required", HTTP_BAD_REQUEST);
    return;
  } else if(request.get_header_value("Accept") == "application/json") {
    pAccept  = "application/json";
  } else if(request.get_header_value("Accept") == "application/cbor") {
    pAccept = "application/cbor";
  } else {
    Error("Unsupported 'Accept' header value", HTTP_BAD_REQUEST);
    return;
  }
  if(request.has_header("Content-Type") == false) {
    Error("'Content-Type' header required", HTTP_BAD_REQUEST);
    return;
  } else if(request.get_header_value("Content-Type") == "application/json") {
    pParams = json::parse(request.body, NULL, false);
    if(pParams.is_discarded()) {
      Error("Malformed JSON content", HTTP_BAD_REQUEST);
      return;
    }
  } else if(request.get_header_value("Content-Type") == "application/cbor") {
    pParams = json::from_cbor(request.body, true, false);
    if(pParams.is_discarded()) {
      Error("Malformed CBOR content", HTTP_BAD_REQUEST);
      return;
    }
  } else {
    Error("Unsupported 'Content-Type' header value", HTTP_BAD_REQUEST);
    return;
  }
  if(request.has_header("Cookie")) {
    string Cookie = request.get_header_value("Cookie");
    auto Begin = Cookie.find("SESSION=");
    if(Begin != string::npos) pCookie = Cookie.substr(Begin + 8, Cookie.find(';', Begin) - Begin - 8);
    if(!pCookie.empty()) {
      Database DB;
      if(DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(pCookie)) &&  DB.FecthRow()) pUserName = DB.ResultString("username");
    }
  }
  if(pUserName.empty() && (request.remote_addr == "127.0.0.1") && pParams.contains("UserName") && pParams["UserName"].is_string()) {
    pUserName = pParams["UserName"];
    pParams.erase("UserName");
  }
}
HTTPRequest::HTTPRequest(const json &params) {
  pResponse = NULL;
  pParams   = params;
  Error("", HTTP_NOT_FOUND);
}
HTTPRequest::~HTTPRequest() {
}
void HTTPRequest::Error(const string &body, const int status) {
  if(pResponse != NULL) {
    pResponse->set_content(body, "text/plain");
    pResponse->status = status;
  } else {
    pStatus = status;
  }
}
void HTTPRequest::Success(const json &body) {
  if(pResponse != NULL) {
    if(pAccept == "application/json") {
      pResponse->set_content(body.dump(), "application/json");
    } else if(pAccept == "application/cbor") {
      vector<std::uint8_t> Buff = json::to_cbor(body);
      pResponse->set_content((char*)Buff.data(), Buff.size(), "application/cbor");
    }
    pResponse->status = HTTP_OK;
  } else {
    pStatus = HTTP_OK;
  }
}
void HTTPRequest::SuccessStreamArray(function<void(function<bool(const json &)>)> func) {
  if(pResponse != NULL) {
    bool UseCBOR = (pAccept == "application/cbor");
    auto Func = [func, UseCBOR](uint64_t offset, DataSink &sink) -> bool {
      uint32_t Items = 0;
      auto Write = [&](const json &content) -> bool {
        if(UseCBOR) {
          vector<uint8_t> Buff = json::to_cbor(content);
          sink.write((char*)Buff.data(), Buff.size());
        } else {
          string Buff = Items ? "," : "";
          Buff.append(content.dump());
          sink.write(Buff.c_str(), Buff.size());
          Items++;
        }
        return sink.is_writable();
      };
      char OpenChar  = UseCBOR ? 0x9F : '[';
      char CloseChar = UseCBOR ? 0xFF : ']';
      sink.write(&OpenChar, 1);
      func(Write);
      sink.write(&CloseChar, 1);
      sink.done();
      return true;
    };
    pResponse->status = HTTP_OK;
    pResponse->set_chunked_content_provider("application/json", Func);
  } else {
    pStatus = HTTP_OK;
  }
}
bool HTTPRequest::CheckParamNumber(const string &name) {
  if((pParams.contains(name) == false) || (pParams[name].is_number() == false)) {
    Error("'" + name + "' param is required", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamString(const string &name) {
  if((pParams.contains(name) == false) || (pParams[name].is_string() == false)) {
    Error("'" + name + "' param is required", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamBool(const string &name) {
  if((pParams.contains(name) == false) || (pParams[name].is_boolean() == false)) {
    Error("'" + name + "' param is required", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamObject(const string &name) {
  if((pParams.contains(name) == false) || (pParams[name].is_object() == false)) {
    Error("'" + name + "' param is required", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamArray(const string &name) {
  if((pParams.contains(name) == false) || (pParams[name].is_array() == false)) {
    Error("'" + name + "' param is required", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamEnum(const string &name, const vector<string> &values) {
  if(CheckParamString(name) == false) return false;
  string Val = pParams[name];
  auto Tmp   = find(values.begin(), values.end(), Val);
  if(Tmp == values.end()) {
    Error("Invalid value '" + Val + "' in param '" + name + "'", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamSet(const string &name, const vector<string> &values, const bool needAll) {
  vector<string> Valid = values;
  if(CheckParamArrayString(name) == false) return false;
  for(auto &Item : pParams[name]) {
    auto Tmp = find(Valid.begin(), Valid.end(), Item);
    if(Tmp == Valid.end()) {
      Error("Invalid value '" + *Tmp + "' in param '" + name + "'", HTTP_BAD_REQUEST);
      return false;
    }
    Valid.erase(Tmp);
  }
  if(needAll && !Valid.empty()) {
    Error("Incomplete param '" + name + "'", HTTP_BAD_REQUEST);
    return false;
  }
  return true;
}
bool HTTPRequest::CheckParamArrayString(const string &name) {
  if((pParams.contains(name) == false) || (pParams[name].is_array() == false)) {
    Error("'" + name + "' param is required", HTTP_BAD_REQUEST);
    return false;
  }
  for(auto &Item : pParams[name]) {
    if(Item.is_string() == false) {
      Error("'" + name + "' param is invalid", HTTP_BAD_REQUEST);
      return false;
    }
  }
  return true;
}
void HTTPRequest::SetHeader(const string &name, const string &value) {
  if(pResponse != NULL) pResponse->set_header(name.c_str(), value.c_str());
}
json HTTPRequest::Params() const {
  return pParams;
}
json HTTPRequest::Param(const string &param) const {
  return pParams[param];
}
string HTTPRequest::Cookie() const {
  return pCookie;
}
string HTTPRequest::UserName() const {
  return pUserName;
}
int HTTPRequest::Status() const {
  return (pResponse != NULL) ? pResponse->status : pStatus;
}