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
// ---------------------------------------------------------------------------------------------------------------------
UserManager::UserManager() {
  HTTPServer::RegisterCallback("getUsers",        GetUsersCallback);
  HTTPServer::RegisterCallback("getUser",         GetUserCallback);
  HTTPServer::RegisterCallback("setUser",         SetUserCallback);
  HTTPServer::RegisterCallback("setUserPassword", SetUserPasswordCallback);
  HTTPServer::RegisterCallback("addUser",         AddUserCallback);
  HTTPServer::RegisterCallback("delUser",         DelUserCallback);
}
UserManager::~UserManager() {
  HTTPServer::UnregisterCallback("getVersion");
  HTTPServer::UnregisterCallback("getUsers");
  HTTPServer::UnregisterCallback("getUser");
  HTTPServer::UnregisterCallback("setUser");
  HTTPServer::UnregisterCallback("setUserPassword");
  HTTPServer::UnregisterCallback("addUser");
  HTTPServer::UnregisterCallback("delUser");
}
void UserManager::GetUsersCallback(const json &params, Response &response) {
  //only 'admin' user can get this information
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('User', username, 'Name', name, 'Email', email, 'TimeZone', time_zone) ORDER BY username ASC)), '[]') AS result FROM users");
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void UserManager::GetUserCallback(const json &params, Response &response) {
  string UserName = params["UserName"];
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string User = params["User"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT json_build_object('User', username, 'Name', name, 'Address', address, 'Company', company, 'Photo', photo, 'Email', email, 'Telegram', telegram, 'TimeZone', time_zone) AS result FROM users WHERE username = " + DB.Bind(User));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void UserManager::SetUserCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Address") == false) || (params["Address"].is_string() == false)) return HTTPServer::FillResponse(response, "'Address' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Company") == false) || (params["Company"].is_string() == false)) return HTTPServer::FillResponse(response, "'Company' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Photo") == false) || (params["Photo"].is_string() == false)) return HTTPServer::FillResponse(response, "'Photo' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Email") == false) || (params["Email"].is_string() == false)) return HTTPServer::FillResponse(response, "'Email' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Telegram") == false) || (params["Telegram"].is_number() == false)) return HTTPServer::FillResponse(response, "'Telegram' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("TimeZone") == false) || (params["TimeZone"].is_string() == false)) return HTTPServer::FillResponse(response, "'TimeZone' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string  UserName = params["UserName"];
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
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void UserManager::SetUserPasswordCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Password") == false) || (params["Password"].is_string() == false)) return HTTPServer::FillResponse(response, "'Password' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string UserName = params["UserName"];
  string User = params["User"];
  string Password = params["Password"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  if(UserName == User) {
    if((params.contains("PrevPassword") == false) || (params["PrevPassword"].is_string() == false)) return HTTPServer::FillResponse(response, "'PrevPassword' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    string PrevPassword = params["PrevPassword"];
    DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(PrevPassword) + ") RETURNING username");
    return HTTPServer::FillResponse(response, "", "text/plain", (DB.FecthRow() == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
  } else {
    DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + ") RETURNING username");
    return HTTPServer::FillResponse(response, "", "text/plain", (DB.FecthRow() == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
  }
}
void UserManager::AddUserCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Address") == false) || (params["Address"].is_string() == false)) return HTTPServer::FillResponse(response, "'Address' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Company") == false) || (params["Company"].is_string() == false)) return HTTPServer::FillResponse(response, "'Company' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Photo") == false) || (params["Photo"].is_string() == false)) return HTTPServer::FillResponse(response, "'Photo' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Email") == false) || (params["Email"].is_string() == false)) return HTTPServer::FillResponse(response, "'Email' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Telegram") == false) || (params["Telegram"].is_number() == false)) return HTTPServer::FillResponse(response, "'Telegram' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("TimeZone") == false) || (params["TimeZone"].is_string() == false)) return HTTPServer::FillResponse(response, "'TimeZone' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Password") == false) || (params["Password"].is_string() == false)) return HTTPServer::FillResponse(response, "'Password' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string  UserName = params["UserName"];
  string  User = params["User"];
  string  Password = params["Password"];
  string  Name = params["Name"];
  string  Address = params["Address"];
  string  Company = params["Company"];
  string  Photo = params["Photo"];
  string  Email = params["Email"];
  int64_t Telegram = params["Telegram"];
  string  TimeZone = params["TimeZone"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  bool Result = DB.ExecSentenceNoResult("INSERT INTO users(username, password, lastlogin, name, address, company, photo, email, telegram, time_zone) VALUES(" + DB.Bind(User) + ", MD5(" + DB.Bind(Password) + "), 0, " + DB.Bind(Name) + ", " + DB.Bind(Address) + ", " + DB.Bind(Company) +  ", " + DB.Bind(Photo) + ", " + DB.Bind(Email) + ", " + DB.Bind(Telegram) + ", " + DB.Bind(TimeZone) + ")");
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void UserManager::DelUserCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string UserName = params["UserName"];
  string User = params["User"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  bool Result = DB.ExecSentenceNoResult("DELETE FROM users WHERE username = " + DB.Bind(User));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
// ---------------------------------------------------------------------------------------------------------------------