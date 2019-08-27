#include "SysSupport.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
bool SysSupport::Registered = SysSupport::Register();
//----------------------------------------------------------------------------------------------------------------------
bool SysSupport::Register() {
  HTTPServer::RegisterCallback("getNews",   SysSupport::GetNewsCallback);
  HTTPServer::RegisterCallback("getUsers",  SysSupport::GetUsersCallback);  
  HTTPServer::RegisterCallback("getUser",   SysSupport::GetUserCallback);
  HTTPServer::RegisterCallback("setUser",   SysSupport::SetUserCallback);
  return true;
}
void SysSupport::GetNewsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(Params["NewsTime"].is_number()) {
    uint64_t Time = Params["NewsTime"];
    req.ExecSentenceAsyncResult("SELECT COALESCE((SELECT json_build_object('NewsTime', newstime, 'Header', header, 'Contents', contents) FROM news WHERE newstime = " + req.Bind(Time) + "), 'null') AS result");
  } else {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Version', '" __BUILD__ "', 'News', array_agg(newstime ORDER BY newstime ASC)) AS result from news");
  }
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void SysSupport::GetUsersCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  req.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('UserName', username, 'Name', name, 'EMail', email, 'TimeZone', time_zone))) AS result FROM users");
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void SysSupport::GetUserCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string UserName = req.UserName();
  if(Params.contains("UserName") && Params["UserName"].is_string()) UserName = Params["UserName"];
  if((req.UserName() != "admin") || (req.UserName() != UserName)) return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  req.ExecSentenceAsyncResult("SELECT json_build_object('UserName', username, 'Name', name, 'Address', address, 'Company', company, 'Photo', photo, 'EMail', email, 'TelegramID', telegram, 'TimeZone', time_zone) AS result FROM users WHERE username = " + req.Bind(UserName));
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void SysSupport::SetUserCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //req.ExecSentenceAsyncResult("UPDATE users SET name = ", 'Address', address, 'Company', company, 'Photo', photo, 'EMail', email, 'TelegramID', telegram, 'Country_Code', country_code) AS result FROM users WHERE username = " + req.Bind(req.UserName()));
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}

//void GetCountriesCallback(HTTPRequest &req) {
//  req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('Country_Code', country_code, 'Name', country, 'Currency', currency, 'TimeZone', timezone))), '[]') as result FROM country");
//  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
//  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
//}
