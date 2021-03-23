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
#include "UserManager.h"
#include "Database.h"
#include "LogFile.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
UserManager::UserManager() {
  LogFile::Info("Starting UserManager...");
  HTTPServer::RegisterCallback("getUsers",        GetUsersCallback,        "API/User/GetUsers.json");
  HTTPServer::RegisterCallback("getUser",         GetUserCallback,         "API/User/GetUser.json");
  HTTPServer::RegisterCallback("setUser",         SetUserCallback,         "API/User/SetUser.json");
  HTTPServer::RegisterCallback("setUserPassword", SetUserPasswordCallback, "API/User/SetUserPassword.json");
  HTTPServer::RegisterCallback("addUser",         AddUserCallback,         "API/User/AddUser.json");
  HTTPServer::RegisterCallback("delUser",         DelUserCallback,         "API/User/DelUser.json");
  LogFile::Info("UserManager started");
}
UserManager::~UserManager() {
  LogFile::Info("Stopping UserManager...");
  HTTPServer::UnregisterCallback("getVersion");
  HTTPServer::UnregisterCallback("getUsers");
  HTTPServer::UnregisterCallback("getUser");
  HTTPServer::UnregisterCallback("setUser");
  HTTPServer::UnregisterCallback("setUserPassword");
  HTTPServer::UnregisterCallback("addUser");
  HTTPServer::UnregisterCallback("delUser");
  LogFile::Info("UserManager stoped");
}
void UserManager::GetUsersCallback(HTTPRequest &req) {/*
  string UserName = req.UserName();
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  auto Func = [UserName](uint64_t offset, DataSink &sink) -> bool {
    uint32_t Items = 0;
    sink.os << "[\n  ";
    Database DB;
    DB.ExecSentenceAsyncResult("SELECT json_build_object('User', username, 'Name', name, 'Email', email, 'TimeZone', time_zone) AS result FROM users ORDER BY username ASC");
    while(DB.FecthRow() && sink.is_writable()) sink.os << ((Items++ != 0) ? ",\n  " + DB.ResultString("result") : DB.ResultString("result"));
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);*/
}
void UserManager::GetUserCallback(HTTPRequest &req) {
  if(!req.CheckParamString("User")) return;
  string User = req.Param("User");
  if((req.UserName() != "admin") && (req.UserName() != User)) return req.Error("", HTTP_FORBIDDEN);
  Database DB;
  if((DB.ExecSentenceAsyncResult("SELECT json_build_object('User', username, 'Name', name, 'Address', address, 'Company', company, 'Photo', photo, 'Email', email, 'Telegram', telegram, 'TimeZone', time_zone) FROM users WHERE username = " + DB.Bind(User)) == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void UserManager::SetUserCallback(HTTPRequest &req) {/*
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Address") == false) || (params["Address"].is_string() == false)) return HTTPServer::FillResponse(response, "'Address' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Company") == false) || (params["Company"].is_string() == false)) return HTTPServer::FillResponse(response, "'Company' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Photo") == false) || (params["Photo"].is_string() == false)) return HTTPServer::FillResponse(response, "'Photo' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Email") == false) || (params["Email"].is_string() == false)) return HTTPServer::FillResponse(response, "'Email' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Telegram") == false) || (params["Telegram"].is_number() == false)) return HTTPServer::FillResponse(response, "'Telegram' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("TimeZone") == false) || (params["TimeZone"].is_string() == false)) return HTTPServer::FillResponse(response, "'TimeZone' param is required", "text/plain", HTTP_BAD_REQUEST);
  string  UserName = req.UserName();
  string  User = params["User"];
  string  Name = params["Name"];
  string  Address = params["Address"];
  string  Company = params["Company"];
  string  Photo = params["Photo"];
  string  Email = params["Email"];
  int64_t Telegram = params["Telegram"];
  string  TimeZone = params["TimeZone"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  bool Result = DB.ExecSentenceNoResult("UPDATE users SET name = " + DB.Bind(Name) + ", address = " + DB.Bind(Address) + ", company = " + DB.Bind(Company) +  ", photo = " + DB.Bind(Photo) + ", email = " + DB.Bind(Email) + ", telegram = " + DB.Bind(Telegram) + ", time_zone = " + DB.Bind(TimeZone) + " WHERE username = " + DB.Bind(User));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);*/
}
void UserManager::SetUserPasswordCallback(HTTPRequest &req) {/*
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Password") == false) || (params["Password"].is_string() == false)) return HTTPServer::FillResponse(response, "'Password' param is required", "text/plain", HTTP_BAD_REQUEST);
  string UserName = req.UserName();
  string User = params["User"];
  string Password = params["Password"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  if(UserName == User) {
    if((params.contains("PrevPassword") == false) || (params["PrevPassword"].is_string() == false)) return HTTPServer::FillResponse(response, "'PrevPassword' param is required", "text/plain", HTTP_BAD_REQUEST);
    string PrevPassword = params["PrevPassword"];
    DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(PrevPassword) + ") RETURNING username");
    return HTTPServer::FillResponse(response, "", "text/plain", (DB.FecthRow() == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
  } else {
    DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + " RETURNING username");
    return HTTPServer::FillResponse(response, "", "text/plain", (DB.FecthRow() == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
  }*/
}
void UserManager::AddUserCallback(HTTPRequest &req) {
  if(!req.CheckParamString("User") || !req.CheckParamString("Name") || !req.CheckParamString("Address") || !req.CheckParamString("Company") || !req.CheckParamString("Photo") || !req.CheckParamString("Email") || !req.CheckParamString("TimeZone") || !req.CheckParamString("Password") || !req.CheckParamNumber("Telegram")) return;
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  string  User     = req.Param("User");
  string  Password = req.Param("Password");
  string  Name     = req.Param("Name");
  string  Address  = req.Param("Address");
  string  Company  = req.Param("Company");
  string  Photo    = req.Param("Photo");
  string  Email    = req.Param("Email");
  int64_t Telegram = req.Param("Telegram");
  string  TimeZone = req.Param("TimeZone");
  Database DB;
  if(DB.ExecSentenceNoResult("INSERT INTO users(username, password, lastlogin, name, address, company, photo, email, telegram, time_zone) VALUES(" + DB.Bind(User) + ", MD5(" + DB.Bind(Password) + "), 0, " + DB.Bind(Name) + ", " + DB.Bind(Address) + ", " + DB.Bind(Company) +  ", " + DB.Bind(Photo) + ", " + DB.Bind(Email) + ", " + DB.Bind(Telegram) + ", " + DB.Bind(TimeZone) + ")") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void UserManager::DelUserCallback(HTTPRequest &req) {
  if(!req.CheckParamString("User")) return;
  string User = req.Param("User");
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  Database DB;
  if(DB.ExecSentenceNoResult("DELETE FROM users WHERE username = " + DB.Bind(User)) == false) req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
// ---------------------------------------------------------------------------------------------------------------------