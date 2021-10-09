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
void UserManager::GetUsersCallback(HTTPRequest &req) {
  string UserName = req.UserName();
  if(UserName != "admin") return req.Error("", HTTP_FORBIDDEN);
  auto Func = [UserName](function<bool(const json &content)> Write) {
    json Headers = {"User", "Name", "Email", "TimeZone"};
    Write(Headers);
    Database DB;
    DB.ExecSentenceAsyncResult("SELECT json_build_array(username, name, email, time_zone) FROM users ORDER BY username ASC");
    while(DB.FecthRow()) {
      if(Write(DB.ResultJSON(0)) == false) return;
    }
  };
  req.SuccessStreamArray(Func);
}
void UserManager::GetUserCallback(HTTPRequest &req) {
  if(!req.CheckParamString("User")) return;
  string User = req.Param("User");
  if((req.UserName() != "admin") && (req.UserName() != User)) return req.Error("", HTTP_FORBIDDEN);
  Database DB;
  if((DB.ExecSentenceAsyncResult("SELECT json_build_object('User', username, 'Name', name, 'Address', address, 'Company', company, 'Photo', photo, 'Email', email, 'Telegram', telegram, 'TimeZone', time_zone) FROM users WHERE username = " + DB.Bind(User)) == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void UserManager::SetUserCallback(HTTPRequest &req) {
  if(!req.CheckParamString("User") || !req.CheckParamString("Name") || !req.CheckParamString("Address") || !req.CheckParamString("Company") || !req.CheckParamString("Photo") || !req.CheckParamString("Email") || !req.CheckParamString("TimeZone") || !req.CheckParamNumber("Telegram")) return;
  if((req.UserName() != "admin") && (req.UserName() != req.Param("User"))) return req.Error("", HTTP_FORBIDDEN);
  string  User     = req.Param("User");
  string  Name     = req.Param("Name");
  string  Address  = req.Param("Address");
  string  Company  = req.Param("Company");
  string  Photo    = req.Param("Photo");
  string  Email    = req.Param("Email");
  int64_t Telegram = req.Param("Telegram");
  string  TimeZone = req.Param("TimeZone");
  Database DB;
  if(DB.ExecSentenceNoResult("UPDATE users SET name = " + DB.Bind(Name) + ", address = " + DB.Bind(Address) + ", company = " + DB.Bind(Company) +  ", photo = " + DB.Bind(Photo) + ", email = " + DB.Bind(Email) + ", telegram = " + DB.Bind(Telegram) + ", time_zone = " + DB.Bind(TimeZone) + " WHERE username = " + DB.Bind(User)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void UserManager::SetUserPasswordCallback(HTTPRequest &req) {
  if(!req.CheckParamString("User") || !req.CheckParamString("Password")) return;
  string User     = req.Param("User");
  string Password = req.Param("Password");
  if((req.UserName() != "admin") && (req.UserName() != User)) return req.Error("", HTTP_FORBIDDEN);
  Database DB;
  if(req.UserName() == User) {
    if(!req.CheckParamString("PrevPassword")) return;
    string PrevPassword = req.Param("PrevPassword");
    if((DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(PrevPassword) + ") RETURNING username") == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else {
    if((DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + " RETURNING username") == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  }
 req.Success(json::object());
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